﻿using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net.Sockets;
<<<<<<< HEAD
=======
using System.Threading.Tasks;
>>>>>>> 894ddd66d53af3bf114b9b2beaf224d63e041928
using MySql.Data.MySqlClient;
using MySql.Data.Types;

namespace Server
{
    public class ConnectHandler
    {
        private CloudPhoneWindow cloudPhoneWindow;
<<<<<<< HEAD
        public TcpListener threadListener;
        public bool isRunning { get; set; }
        public String ip;
         TcpClient client;
         NetworkStream ns;
=======
        private int size;
        public TcpListener threadListener;
        public bool isRunning { get; set; }
>>>>>>> 894ddd66d53af3bf114b9b2beaf224d63e041928

        public ConnectHandler(CloudPhoneWindow c)
        {
            isRunning = true;
            cloudPhoneWindow = c;
            cloudPhoneWindow.Invoke(cloudPhoneWindow._logMSG, "info", "클라이언트 생성, ConnectHandler");
        }

        public void clientHandler()
        {
            try
            {
<<<<<<< HEAD
                client = threadListener.AcceptTcpClient();
                ns = client.GetStream();
=======
                TcpClient client = threadListener.AcceptTcpClient();
                NetworkStream ns = client.GetStream();
                byte[] buffer = new byte[1024];
                int receiveLength = 0;
>>>>>>> 894ddd66d53af3bf114b9b2beaf224d63e041928

                while (isRunning)
                {
                    try
                    {
<<<<<<< HEAD
                        System.IO.StreamReader sr = new System.IO.StreamReader(ns);

                        String receive = "";

                        receive = sr.ReadLine();
                        cloudPhoneWindow.Invoke(cloudPhoneWindow._logMSG, "info", "REceive  MSG " + receive);
                        
                        UnPackingMessage(receive);

                        sr.Close();
=======
                        int totalLength = 0;

                        ns.Read(buffer, 0, buffer.Length);
                        int fileLength = BitConverter.ToInt32(buffer, 0);
                        var stream = new MemoryStream();
                        while (totalLength < fileLength)
                        {
                            receiveLength = ns.Read(buffer, 0, buffer.Length);
                            stream.Write(buffer, 0, receiveLength);
                            totalLength += receiveLength;
                        }

                        String receive = Encoding.UTF8.GetString(buffer, 0, totalLength);
                        
                        /*
                         *  receive받은 String으로 Unpacking함 
                         */

                        UnPackingMessage(receive);
>>>>>>> 894ddd66d53af3bf114b9b2beaf224d63e041928

                    }
                    catch (Exception e)
                    {
<<<<<<< HEAD
                        cloudPhoneWindow.Invoke(cloudPhoneWindow._logMSG, "error", "clientHandler : " + e.Message);
=======
                        cloudPhoneWindow.Invoke(cloudPhoneWindow._logMSG, "clientHandler : " + e.Message);
>>>>>>> 894ddd66d53af3bf114b9b2beaf224d63e041928
                        isRunning = false;
                    }
                }
                ns.Close();
                client.Close();

            }
            catch(Exception e)
            {
                cloudPhoneWindow.Invoke(cloudPhoneWindow._logMSG, "error", "clientHandler : " + e.Message);
                isRunning = false;
            }
        }

<<<<<<< HEAD
        // Client에 BackGround 파일 전송(.png)
        private void SendBG()
        {


        }

        // Client로 메시지 전달
        private void SendMsg(String msg)
        {
            if (ns != null && ns.CanWrite == true)
            {
                System.IO.StreamWriter sw = new System.IO.StreamWriter(ns);
                sw.WriteLine(msg);
                sw.Flush();
                sw.Close();
            }
=======

        // Client로 값 전달
        private void SendData(String msg)
        {

>>>>>>> 894ddd66d53af3bf114b9b2beaf224d63e041928
        }

        // Client로부터 온 메시지 Unpack
        private void UnPackingMessage(String str)
        {
            String[] strings = str.Split('/');

            // Unpacking해서 메시지 헤더에 따라서 돌린다.
            // 메시지 구조  :  Type / Size / Data ( Data는 '쉼표'로 구분 ',')
            // ACTION_DOWN, ACTION_MOVE, ACTION_UP, ACTION_POINTER_DOWN,
            // ACTION_POINTER_UP, KEYCODE_HOME, KEYCODE_VOLUME_DOWN, KEYCODE_VOLUME_UP, KEYCODE_POWER, GPS, GYRO, BATTERY

            switch (strings[0])
            {
                case "0": // Login 메시지가 들어오면, 있는 ID인지 검사하도록  CloudPhoneWindow에 있는 CheckClientID를 실행시킨다.
<<<<<<< HEAD
                    ip = strings[2];
                    cloudPhoneWindow.Invoke(cloudPhoneWindow._logMSG, "info", "ip : " + ip);
                    //cloudPhoneWindow.ClientLogin(strings[2]);
                    break;

                case "1": // AVD MSG ( 생성 / 삭제 / 실행 / 종료 ) 
                    cloudPhoneWindow.DecideAVDMsg(strings[2]);
=======
                    cloudPhoneWindow.ClientLogin(strings[2]);
                    break;

                case "1": // AVD MSG ( 생성 / 삭제 / 실행 / 종료 ) 

                    cloudPhoneWindow.DecideAVDMsg(strings[2]);
                    
>>>>>>> 894ddd66d53af3bf114b9b2beaf224d63e041928
                    break;

                case "2": // ACTION

                    break;

                case "3": // KEYCODE VALUE

                    break;

                case "4": // GPS VALUE

                    break;

                case "5": // GYRO VALUE

                    break;

                case "6": // BATTERY VALUE

                    break;

                case "7": // Client LogOut
<<<<<<< HEAD
                   // cloudPhoneWindow.ClientLogout(strings[2]);
=======
                    cloudPhoneWindow.ClientLogout(strings[2]);
>>>>>>> 894ddd66d53af3bf114b9b2beaf224d63e041928
                    break;
                default:

                    break;
            }


        }

        // Client로 보낼 메시지를 Pack한다.
        private String PackingMessage(params String[] strings)
        {
            String result = "";

            foreach (String msg in strings)
            {
                result += msg;
                result += '/';
            }

            return result;
        }
    }
}
