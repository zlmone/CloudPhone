package org.secmem232.cloudphone.network;

import org.secmem232.cloudphone.util.ConvertUtil;

public class PacketHeader {
	public static final int LENGTH = 10;
	public static final int OPCODE_LENGTH = 2;
	public static final int PAYLOAD_LENGTH = 8;
	
	public class OpCode{
		public static final int INVALID = -1;
		public static final int INFO_SEND = 1;
		public static final int DATA_SEND = 2;
		public static final int SCREEN_SEND_REQUESTED = 3;
		public static final int SCREEN_STOP_REQUESTED = 4;
		public static final int SCREEN_ON_STATE_INFO = 5;
		public static final int SCREEN_OFF_STATE_INFO = 6;
	}
	
	private int opCode = OpCode.INVALID;
	private int payloadLength = 0;
	
	private static byte[] opCodeBuffer = new byte[OPCODE_LENGTH];
	private static byte[] packetSizeBuffer = new byte[PAYLOAD_LENGTH];
	private static byte[] heaerBuffer = new byte[OPCODE_LENGTH + PAYLOAD_LENGTH];
	
	private PacketHeader(){
		
	}
	
	public PacketHeader(int opCode, int dataLength){
		this.opCode = opCode;
		this.payloadLength = dataLength;
	}
	
	@Override
	public String toString(){
		return String.format("%2d%8d", opCode, payloadLength + LENGTH);
	}
	
	public byte[] asByteArray(){
		ConvertUtil.itoa(opCode, heaerBuffer, OPCODE_LENGTH, 0);
		ConvertUtil.itoa(payloadLength+LENGTH, heaerBuffer, PAYLOAD_LENGTH, OPCODE_LENGTH);
		return heaerBuffer;
	}
	
	private static int ByteToInt(byte [] data){
		int result = 0;
		for(int i=0; i<data.length; i++){
			if(data[i] == ' ')
				continue;
			result = result * 10 + (data[i]-'0');
		}
		return result;
	}
	
	public static PacketHeader parse(byte[] rawData){
		PacketHeader header = new PacketHeader();
		System.arraycopy(rawData, 0, opCodeBuffer, 0, OPCODE_LENGTH);
		header.setOpCode(ByteToInt(opCodeBuffer));
		System.arraycopy(rawData, OPCODE_LENGTH, packetSizeBuffer, 0, PAYLOAD_LENGTH);				
		header.setPayloadLength(ByteToInt(packetSizeBuffer)-PacketHeader.LENGTH);
		return header;
	}

	public int getOpCode() {
		return opCode;
	}

	public void setOpCode(int opCode) {
		this.opCode = opCode;
	}

	public int getPayloadLength() {
		return payloadLength;
	}

	public void setPayloadLength(int payloadLength) {
		this.payloadLength = payloadLength;
	}
	
	public int getPacketLength() {
		return LENGTH + payloadLength;
	}
	
}
