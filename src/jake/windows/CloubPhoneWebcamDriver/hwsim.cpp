
#include "avshws.h"


/*************************************************/
KDEFERRED_ROUTINE SimulatedInterrupt;

void SimulatedInterrupt(IN PKDPC Dpc, IN PVOID DeferredContext, IN PVOID SystemArg1, IN PVOID SystemArg2)
{
	CHardwareSimulation* HardwareSim = (CHardwareSimulation*)DeferredContext;
	HardwareSim->FakeHardware();
}


/**************************************************************************

PAGEABLE CODE

**************************************************************************/

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif // ALLOC_PRAGMA


/*++

Routine Description:

Construct a hardware simulation

Arguments:

HardwareSink -
The hardware sink interface.  This is used to trigger
fake interrupt service routines from.

Return Value:

Success / Failure

--*/
CHardwareSimulation::CHardwareSimulation(IN IHardwareSink *HardwareSink) : m_HardwareSink(HardwareSink), m_ScatterGatherMappingsMax(SCATTER_GATHER_MAPPINGS_MAX)
{
	PAGED_CODE();
	//
	// Initialize the DPC's, timer's, and locks necessary to simulate
	// this capture hardware.
	//
	KeInitializeDpc(&m_IsrFakeDpc, SimulatedInterrupt, this);
	KeInitializeEvent(&m_HardwareEvent, SynchronizationEvent, FALSE);
	KeInitializeTimer(&m_IsrTimer);
	KeInitializeSpinLock(&m_ListLock);
}

/*************************************************/

/*++

Routine Description:

Initialize the hardware simulation

Arguments:

HardwareSink -
The hardware sink interface.  This is what ISR's will be
triggered through.

Return Value:

A fully initialized hardware simulation or NULL if the simulation
could not be initialized.

--*/
CHardwareSimulation * CHardwareSimulation::Initialize(IN KSOBJECT_BAG Bag, IN IHardwareSink *HardwareSink)
{
	PAGED_CODE();
	CHardwareSimulation *HwSim = new (NonPagedPool)CHardwareSimulation(HardwareSink);
	return HwSim;
}

/*************************************************/

/*++

Routine Description:

Start the hardware simulation.  This will kick the interrupts on,
begin issuing DPC's, filling in capture information, etc...
We keep track of starvation starting at this point.

Arguments:

ImageSynth -
The image synthesizer to use to generate pictures to display
on the capture buffer.

TimePerFrame -
The time per frame...  we issue interrupts this often.

Width -
The image width

Height -
The image height

ImageSize -
The size of the image.  We allocate a temporary scratch buffer
based on this size to fake hardware.

Return Value:

Success / Failure (typical failure will be out of memory on the
scratch buffer, etc...)

--*/
NTSTATUS CHardwareSimulation::Start(IN CImageSynthesizer *ImageSynth, IN LONGLONG TimePerFrame, IN ULONG Width, IN ULONG Height, IN ULONG ImageSize)
{
	PAGED_CODE();
	NTSTATUS Status = STATUS_SUCCESS;
	m_ImageSynth = ImageSynth;
	m_TimePerFrame = TimePerFrame;
	m_ImageSize = ImageSize;
	m_Height = Height;
	m_Width = Width;

	InitializeListHead(&m_ScatterGatherMappings);
	m_NumMappingsCompleted = 0;
	m_ScatterGatherMappingsQueued = 0;
	m_NumFramesSkipped = 0;
	m_InterruptTime = 0;

	KeQuerySystemTime(&m_StartTime);

	//
	// Allocate a scratch buffer for the synthesizer.
	//
	m_SynthesisBuffer = reinterpret_cast <PUCHAR> (ExAllocatePoolWithTag(NonPagedPool, m_ImageSize, AVSHWS_POOLTAG));

	if (!m_SynthesisBuffer) {
		Status = STATUS_INSUFFICIENT_RESOURCES;
	}

	//
	// If everything is ok, start issuing interrupts.
	//
	if (NT_SUCCESS(Status)) {

		//
		// Initialize the entry lookaside.
		//
		ExInitializeNPagedLookasideList(&m_ScatterGatherLookaside, NULL, NULL, 0, sizeof (SCATTER_GATHER_ENTRY), 'nEGS', 0);

		//
		// Set up the synthesizer with the width, height, and scratch buffer.
		//
		m_ImageSynth->SetImageSize(m_Width, m_Height);
		m_ImageSynth->SetBuffer(m_SynthesisBuffer);

		LARGE_INTEGER NextTime;
		NextTime.QuadPart = m_StartTime.QuadPart + m_TimePerFrame;

		m_HardwareState = HardwareRunning;
		KeSetTimer(&m_IsrTimer, NextTime, &m_IsrFakeDpc);
	}
	return Status;
}

/*************************************************/

/*++

Routine Description:

Pause the hardware simulation...  When the hardware simulation is told
to pause, it stops issuing interrupts, etc...  but it does not reset
the counters

Arguments:

Pausing -
Indicates whether the hardware is pausing or not.

TRUE -
Pause the hardware

FALSE -
Unpause the hardware from a previous pause


Return Value:

Success / Failure

--*/
NTSTATUS CHardwareSimulation::Pause(BOOLEAN Pausing)
{
	PAGED_CODE();
	if (Pausing && m_HardwareState == HardwareRunning) {
		//
		// If we were running, stop completing mappings, etc...
		//
		m_StopHardware = TRUE;
		KeWaitForSingleObject(&m_HardwareEvent, Suspended, KernelMode, FALSE, NULL);
		ASSERT(m_StopHardware == FALSE);
		m_HardwareState = HardwarePaused;
	}
	else if (!Pausing && m_HardwareState == HardwarePaused) {
		//
		// For unpausing the hardware, we need to compute the relative time
		// and restart interrupts.
		//
		LARGE_INTEGER UnpauseTime;
		KeQuerySystemTime(&UnpauseTime);
		m_InterruptTime = (ULONG)((UnpauseTime.QuadPart - m_StartTime.QuadPart) / m_TimePerFrame);
		UnpauseTime.QuadPart = m_StartTime.QuadPart + (m_InterruptTime + 1) * m_TimePerFrame;
		m_HardwareState = HardwareRunning;
		KeSetTimer(&m_IsrTimer, UnpauseTime, &m_IsrFakeDpc);
	}
	return STATUS_SUCCESS;
}

/**************************************************************************

LOCKED CODE

**************************************************************************/

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif // ALLOC_PRAGMA

/*++

Routine Description:

Stop the hardware simulation....  Wait until the hardware simulation
has successfully stopped and then return.

Arguments:

None

Return Value:

Success / Failure

--*/
NTSTATUS CHardwareSimulation::Stop()
{
	KIRQL Irql;
	//
	// If the hardware is told to stop while it's running, we need to
	// halt the interrupts first.  If we're already paused, this has
	// already been done.
	//
	if (m_HardwareState == HardwareRunning) {
		m_StopHardware = TRUE;
		KeWaitForSingleObject(&m_HardwareEvent, Suspended, KernelMode, FALSE, NULL);
		ASSERT(m_StopHardware == FALSE);
	}

	m_HardwareState = HardwareStopped;
	//
	// The image synthesizer may still be around.  Just for safety's
	// sake, NULL out the image synthesis buffer and toast it.
	//
	m_ImageSynth->SetBuffer(NULL);

	if (m_SynthesisBuffer) {
		ExFreePool(m_SynthesisBuffer);
		m_SynthesisBuffer = NULL;
	}

	//
	// Protect the S/G list
	//
	KeAcquireSpinLock(&m_ListLock, &Irql);
	// 
	// Free S/G buffer 
	// 
	// 
	while (m_ScatterGatherMappingsQueued > 0) {
		LIST_ENTRY *listEntry = RemoveHeadList(&m_ScatterGatherMappings);
		m_ScatterGatherMappingsQueued--;
		PSCATTER_GATHER_ENTRY SGEntry = reinterpret_cast <PSCATTER_GATHER_ENTRY> (CONTAINING_RECORD(listEntry, SCATTER_GATHER_ENTRY, ListEntry));
		// 
		// Release the scatter / gather entry back to our lookaside. 
		// 
		ExFreeToNPagedLookasideList(&m_ScatterGatherLookaside, reinterpret_cast <PVOID> (SGEntry));
	}
	m_NumMappingsCompleted = 0;
	m_ScatterGatherBytesQueued = 0;
	//
	// Delete the scatter / gather lookaside for this run.
	//
	ExDeleteNPagedLookasideList(&m_ScatterGatherLookaside);
	KeReleaseSpinLock(&m_ListLock, Irql);
	return STATUS_SUCCESS;
}

/*++

Routine Description:

Read the number of scatter / gather mappings which have been
completed (TOTAL NUMBER) since the last reset of the simulated
hardware

Arguments:

None

Return Value:

Total number of completed mappings.

--*/
ULONG CHardwareSimulation::ReadNumberOfMappingsCompleted()
{
	//
	// Don't care if this is being updated this moment in the DPC...  I only
	// need a number to return which isn't too great (too small is ok).
	// In real hardware, this wouldn't be done this way anyway.
	//
	return m_NumMappingsCompleted;
}

/*************************************************/

/*++

Routine Description:

Program the scatter gather mapping list.  This shoves a bunch of
entries on a list for access during the fake interrupt.  Note that
we have physical addresses here only for simulation.  We really
access via the virtual address....  although we chunk it into multiple
buffers to more realistically simulate S/G

Arguments:

Buffer -
The virtual address of the buffer mapped by the mapping list

Mappings -
The KSMAPPINGS array corresponding to the buffer

MappingsCount -
The number of mappings in the mappings array

MappingStride -
The mapping stride used in initialization of AVStream DMA

Return Value:

Number of mappings actually inserted.

--*/
ULONG CHardwareSimulation::ProgramScatterGatherMappings(IN PUCHAR *Buffer, IN PKSMAPPING Mappings, IN ULONG MappingsCount, IN ULONG MappingStride)
{
	KIRQL Irql;
	ULONG MappingsInserted = 0;
	//
	// Protect our S/G list with a spinlock.
	//
	KeAcquireSpinLock(&m_ListLock, &Irql);
	//
	// Loop through the scatter / gather list and break the buffer up into
	// chunks equal to the scatter / gather mappings.  Stuff the virtual
	// addresses of these chunks on a list somewhere.  We update the buffer
	// pointer the caller passes as a more convenient way of doing this.
	//
	// If I could just remap physical in the list to virtual easily here,
	// I wouldn't need to do it.
	//
#if defined(_WIN64)
	do
	{
		PSCATTER_GATHER_ENTRY Entry = reinterpret_cast <PSCATTER_GATHER_ENTRY> (ExAllocateFromNPagedLookasideList(&m_ScatterGatherLookaside));

		if (!Entry) {
			break;
		}
		Entry->Virtual = *Buffer;
		Entry->ByteCount = MappingsCount;

		//
		// Move forward a specific number of bytes in chunking this into
		// mapping sized va buffers.
		//
		*Buffer += MappingsCount;
		Mappings = reinterpret_cast <PKSMAPPING> ((reinterpret_cast <PUCHAR> (Mappings)+MappingStride));

		InsertTailList(&m_ScatterGatherMappings, &(Entry->ListEntry));
		MappingsInserted = MappingsCount;
		m_ScatterGatherMappingsQueued++;
		m_ScatterGatherBytesQueued += MappingsCount;
	} while (FALSE);

#elif defined(_X86_)
	for (ULONG MappingNum = 0; MappingNum < MappingsCount && m_ScatterGatherMappingsQueued < m_ScatterGatherMappingsMax; MappingNum++) {
		PSCATTER_GATHER_ENTRY Entry = reinterpret_cast <PSCATTER_GATHER_ENTRY> (ExAllocateFromNPagedLookasideList(&m_ScatterGatherLookaside));
		if (!Entry) {
			break;
		}
		Entry->Virtual = *Buffer;
		Entry->ByteCount = Mappings->ByteCount;

		//
		// Move forward a specific number of bytes in chunking this into
		// mapping sized va buffers.
		//
		*Buffer += Entry->ByteCount;
		Mappings = reinterpret_cast <PKSMAPPING> ((reinterpret_cast <PUCHAR> (Mappings)+MappingStride));

		InsertTailList(&m_ScatterGatherMappings, &(Entry->ListEntry));
		MappingsInserted++;
		m_ScatterGatherMappingsQueued++;
		m_ScatterGatherBytesQueued += Entry->ByteCount;
	}
#endif
	KeReleaseSpinLock(&m_ListLock, Irql);
	return MappingsInserted;
}

/*************************************************/

/*++

Routine Description:

The hardware has synthesized a buffer in scratch space and we're to
fill scatter / gather buffers.

Arguments:

None

Return Value:

Success / Failure

--*/
NTSTATUS CHardwareSimulation::FillScatterGatherBuffers()
{

	//
	// We're using this list lock to protect our scatter / gather lists instead
	// of some hardware mechanism / KeSynchronizeExecution / whatever.
	//
	KeAcquireSpinLockAtDpcLevel(&m_ListLock);

	PUCHAR Buffer = reinterpret_cast <PUCHAR> (m_SynthesisBuffer);
	ULONG BufferRemaining = m_ImageSize;

	//
	// For simplification, if there aren't enough scatter / gather buffers
	// queued, we don't partially fill the ones that are available.  We just
	// skip the frame and consider it starvation.
	//
	// This could be enforced by only programming scatter / gather mappings
	// for a buffer if all of them fit in the table also...
	//
	while (BufferRemaining && m_ScatterGatherMappingsQueued > 0 && m_ScatterGatherBytesQueued >= BufferRemaining) {
		LIST_ENTRY *listEntry = RemoveHeadList(&m_ScatterGatherMappings);
		m_ScatterGatherMappingsQueued--;

		PSCATTER_GATHER_ENTRY SGEntry = reinterpret_cast <PSCATTER_GATHER_ENTRY> (CONTAINING_RECORD(listEntry, SCATTER_GATHER_ENTRY, ListEntry));

		//
		// Since we're software, we'll be accessing this by virtual address...
		//
		ULONG BytesToCopy = (BufferRemaining < SGEntry->ByteCount) ? BufferRemaining : SGEntry->ByteCount;

		RtlCopyMemory(SGEntry->Virtual, Buffer, BytesToCopy);

		BufferRemaining -= BytesToCopy;
		Buffer += BytesToCopy;
		m_NumMappingsCompleted++;
		m_ScatterGatherBytesQueued -= SGEntry->ByteCount;

		//
		// Release the scatter / gather entry back to our lookaside.
		//
		ExFreeToNPagedLookasideList(&m_ScatterGatherLookaside, reinterpret_cast <PVOID> (SGEntry));
	}

	KeReleaseSpinLockFromDpcLevel(&m_ListLock);

	if (BufferRemaining) return STATUS_INSUFFICIENT_RESOURCES;
	else return STATUS_SUCCESS;

}

/*************************************************/

/*++

Routine Description:

Simulate an interrupt and what the hardware would have done in the
time since the previous interrupt.

Arguments:

None

Return Value:

None

--*/
void CHardwareSimulation::FakeHardware()
{
	m_InterruptTime++;

	//
	// The hardware can be in a pause state in which case, it issues interrupts
	// but does not complete mappings.  In this case, don't bother synthesizing
	// a frame and doing the work of looking through the mappings table.
	//
	if (m_HardwareState == HardwareRunning) {

		//
		// Generate a "time stamp" just to overlay it onto the capture image.
		// It makes it more exciting than bars that do nothing.
		//
		LONGLONG PtsRel = ((m_InterruptTime + 1) * m_TimePerFrame);
		ULONG Min = (ULONG)(PtsRel / 600000000);
		ULONG RemMin = (ULONG)(PtsRel % 600000000);
		ULONG Sec = (ULONG)(RemMin / 10000000);
		ULONG RemSec = (ULONG)(RemMin % 10000000);
		ULONG Hund = (ULONG)(RemSec / 100000);

		//
		// Synthesize a buffer in scratch space.
		//        		
		//m_ImageSynth -> SynthesizeBars ();

		CHAR Text[256];
		Text[0] = '\0';
		(void)RtlStringCbPrintfA(Text, sizeof(Text), "%ld:%02ld.%02ld", Min, Sec, Hund);

		m_ImageSynth->setImage();

		//
		// Overlay a clock onto the scratch space image.
		//
		//m_ImageSynth -> OverlayText (
		//    POSITION_CENTER,
		//    (m_Height - 28),
		//    1,
		//    Text,
		//    BLACK,	
		//    WHITE
		//    );

		////
		//// Overlay a counter of skipped frames onto the scratch image.
		////
		//(void) RtlStringCbPrintfA(Text, sizeof(Text), "Skipped: %ld", m_NumFramesSkipped);
		//m_ImageSynth -> OverlayText (
		//    10,
		//    10,
		//    1,
		//    Text,
		//    TRANSPARENT,
		//    BLUE
		//    );

		//
		// Fill scatter gather buffers
		//
		if (!NT_SUCCESS(FillScatterGatherBuffers())) {
			InterlockedIncrement(PLONG(&m_NumFramesSkipped));
		}

	}

	//
	// Issue an interrupt to our hardware sink.  This is a "fake" interrupt.
	// It will occur at DISPATCH_LEVEL.
	//
	m_HardwareSink->Interrupt();

	//
	// Reschedule the timer if the hardware isn't being stopped.
	//
	if (!m_StopHardware) {
		//
		// Reschedule the timer for the next interrupt time.
		//
		LARGE_INTEGER NextTime;
		NextTime.QuadPart = m_StartTime.QuadPart + (m_TimePerFrame * (m_InterruptTime + 1));
		KeSetTimer(&m_IsrTimer, NextTime, &m_IsrFakeDpc);
	}
	else {
		//
		// If someone is waiting on the hardware to stop, raise the stop
		// event and clear the flag.
		//
		m_StopHardware = FALSE;
		KeSetEvent(&m_HardwareEvent, IO_NO_INCREMENT, FALSE);
	}
}
