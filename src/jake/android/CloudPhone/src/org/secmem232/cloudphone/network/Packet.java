package org.secmem232.cloudphone.network;

public class Packet {
	public static final int MAX_LENGTH = 4096 * 10;
	public static final int MAX_PAYLOAD_LENGTH = MAX_LENGTH - PacketHeader.LENGTH;
	
	private PacketHeader header;
	
	private static byte[] packetBuffer = new byte[MAX_LENGTH];
	private static byte[] payloadBuffer = new byte[MAX_PAYLOAD_LENGTH];
	private byte[] payload;
	
	protected Packet(){
	
	}
	
	public Packet(int opCode, byte[] data, int dataLength){
		header = new PacketHeader(opCode, dataLength);
		payload = data;
	}
	
	public static Packet parse(byte[] rawPacket){
		Packet packet = new Packet();		
		// Get header
		packet.setHeader(PacketHeader.parse(rawPacket));		
		
		int payloadLength = packet.getHeader().getPayloadLength();

		// Get data (payload)
		System.arraycopy(rawPacket, PacketHeader.LENGTH, payloadBuffer, 0, payloadLength);
		packet.setPayload(payloadBuffer);		
		// Packet parsing has done.
		return packet;
	}
	
	public byte[] asByteArray(){		
		if(header==null)
			throw new IllegalStateException("Packet header has not been set.");
				
		// Append header
		byte[] headerData = header.asByteArray();
		System.arraycopy(headerData, 0, packetBuffer, 0, PacketHeader.LENGTH);
		
		// Append payload
		if(payload!=null)			
			System.arraycopy(payload, 0, packetBuffer, PacketHeader.LENGTH, payload.length);
		return packetBuffer;
	}
	
	public int getOpcode(){
		return header.getOpCode();
	}

	public PacketHeader getHeader() {
		return header;
	}

	public void setHeader(PacketHeader header) {
		this.header = header;
	}

	public byte[] getPayload() {
		return payload;
	}

	public void setPayload(byte[] payload) {
		this.payload = payload;
	}
}
