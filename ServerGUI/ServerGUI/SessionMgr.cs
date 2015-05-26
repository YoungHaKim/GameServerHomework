using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ProtoBuf;
using MyPacket;
using System.Net.Sockets;
using System.Net;
using System.IO;
using System.Threading;
using System.Windows.Forms;

namespace ServerGUI
{
    public delegate void RecvProtoDelegate(object deserializedProtoObject, object protoObjectTypeEnum);

    public class SessionMgr
    {
        public SessionMgr()
        {
            mTcpClient = new TcpClient();
        }

        public void Connect(string addr, int port)
        {
            if (Running)
            {
                Running = false;
                mTcpClient.Close();
                mTcpClient = new TcpClient();
            }

            mIPEndPoint = new IPEndPoint(IPAddress.Parse(addr), port);
            try
            {
                mTcpClient.Connect(mIPEndPoint);
            }
            catch (SocketException ex)
            {
                MessageBox.Show(ex.Message);                
            }
           
            if (mTcpClient.Connected)
            {
                Running = true;
                // start listen loop
                ThreadStart ts = new ThreadStart(ListenLoop);
                mListenThread = new Thread(ts);
                mListenThread.Start();
            }
        }        

        public void SendPacket(MyPacket.MessageType type, global::ProtoBuf.IExtensible packet)
        {
            if (!Running || !mTcpClient.Connected)
            {
                Console.WriteLine("Connection invalid can't sent packet");
                return;
            }

            BinaryWriter writer = new BinaryWriter(mTcpClient.GetStream());
            
            MemoryStream memory_stream = new MemoryStream();
            ProtoBuf.Serializer.Serialize(memory_stream, packet);
            byte[] byte_array = memory_stream.ToArray();
            
            writer.Write((short)byte_array.Length);
            writer.Write((short)(type));
            writer.Write(byte_array);
            writer.Flush();
        }

        public void SubscribeRecvEvent(RecvProtoDelegate func)
        {
            OnProtoRecvEvent += func;
        }
        public void DesubscribeRecvEvent(RecvProtoDelegate func)
        {
            OnProtoRecvEvent -= func;
        }

        internal void Disconnect()
        {
            Running = false;
            mTcpClient.Close();
        }

        private void ListenLoop()
        {
            NetworkStream stream = mTcpClient.GetStream();
            BinaryReader reader = new BinaryReader(stream);

            while (Running)
            {
                if (stream.DataAvailable == true)
                {
                    int size = reader.ReadInt16();
                    short rawType = reader.ReadInt16();
                    MyPacket.MessageType type = (MyPacket.MessageType)(rawType);

                    if (size < 0) continue;

                    byte[] bytes = reader.ReadBytes(size);

                    if (bytes.Length == size)
                    {
                        MemoryStream memory_stream = new MemoryStream();
                        memory_stream.Write(bytes, 0, size);
                        memory_stream.Position = 0;

                        switch (type)
                        {
                            case MyPacket.MessageType.PKT_SC_FEED:
                                {
                                    MyPacket.Feed inPacket = ProtoBuf.Serializer.Deserialize<MyPacket.Feed>(memory_stream);
                                    if (OnProtoRecvEvent != null)
                                    {
                                        OnProtoRecvEvent((object)inPacket, (object)type);
                                    }
                                } break;
                            
                            case MessageType.PKT_CS_SERVER_STATUS:
                            case MyPacket.MessageType.PKT_SC_SERVER_STATUS:
                                {
                                    MyPacket.ServerStatus inPacket = ProtoBuf.Serializer.Deserialize<MyPacket.ServerStatus>(memory_stream);
                                    if (OnProtoRecvEvent != null)
                                    {
                                        OnProtoRecvEvent((object)inPacket, (object)type);
                                    }
                                }
                                break;

                            case MessageType.PKT_SC_LOGIN:
                                {
                                    MyPacket.LoginResult inPacket = ProtoBuf.Serializer.Deserialize<MyPacket.LoginResult>(memory_stream);
                                    if (OnProtoRecvEvent != null)
                                    {
                                        OnProtoRecvEvent((object)inPacket, (object)type);
                                    }
                                }
                                break;

                            default:
                                Console.WriteLine("Unhandled packet type {0}", type.ToString());
                                break;
                        }
                    }
                }
            }
        }

        Thread mListenThread;
        IPEndPoint mIPEndPoint;
        TcpClient mTcpClient;

        public bool Running { get; set; }
        public RecvProtoDelegate OnProtoRecvEvent;

        
    }
}
