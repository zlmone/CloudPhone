package org.secmem232.cloudphone.network;

import java.io.IOException;
import java.io.OutputStream;

import org.secmem232.cloudphone.network.PacketHeader.OpCode;
import org.secmem232.cloudphone.util.ConvertUtil;


public class CameraSender extends PacketSender {
	private static final int MAXDATASIZE = Packet.MAX_LENGTH - PacketHeader.LENGTH;

	private static final int JPGINFOLENGTH = 10;
	private static final int ORIENTATION_INFO_LENGTH = 1;

	private byte[] sendBuffer = new byte[MAXDATASIZE];
	private byte[] jpgSizeInfo = new byte[JPGINFOLENGTH];

	public CameraSender(OutputStream out){
		super(out);		
	}

	public void screenTransmission(byte[] jpgData, int orientation, int jpgSize) throws IOException{
		int jpgTotalSize = jpgSize;
		int transmittedSize = 0;		

		jpgSizeInfo[0] = (byte)orientation;
		int length = ConvertUtil.itoa(jpgTotalSize, jpgSizeInfo, 1)+ORIENTATION_INFO_LENGTH;

		Packet jpgInfoPacket = new Packet(OpCode.JPGINFO_SEND, jpgSizeInfo, length);

		send(jpgInfoPacket);

		//Next send jpg data to host
		while(jpgTotalSize > transmittedSize){
			int CurTransSize = (jpgTotalSize-transmittedSize) > MAXDATASIZE ? 
					MAXDATASIZE : (jpgTotalSize-transmittedSize);
			System.arraycopy(jpgData, transmittedSize, sendBuffer, 0, CurTransSize);
			transmittedSize += CurTransSize;

			Packet jpgDataPacket = new Packet(OpCode.JPGDATA_SEND, sendBuffer, CurTransSize);

			send(jpgDataPacket);
		}		
	}
}