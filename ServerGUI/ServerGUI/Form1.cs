using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using ProtoBuf;
using System.IO;
using System.Threading;

namespace ServerGUI
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();

        }

        private void button_connect_Click(object sender, EventArgs e)
        {
            mSessionMgr.Connect(textBox_ip.Text, (int)numericUpDown_port.Value);
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            mProductDict = new Dictionary<string, FeedClass>();
            mFeedBindingList = new BindingList<FeedClass>();
            mSessionMgr = new SessionMgr();
            mSessionMgr.SubscribeRecvEvent(OnProtoRecv);
            
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            mSessionMgr.Disconnect();
        }

        private void OnProtoRecv(object deserializedProtoObject, object protoObjectTypeEnum)
        {
            if (deserializedProtoObject == null) return;
            if (protoObjectTypeEnum == null) return;

            MyPacket.MessageType msgType = (MyPacket.MessageType)protoObjectTypeEnum;


                switch (msgType)
                {
                    case MyPacket.MessageType.PKT_CS_LOGIN:
                        break;
                    case MyPacket.MessageType.PKT_SC_LOGIN:
                        break;
                    case MyPacket.MessageType.PKT_CS_CHAT:
                        break;
                    case MyPacket.MessageType.PKT_SC_CHAT:
                        break;
                    case MyPacket.MessageType.PKT_CS_MOVE:
                        break;
                    case MyPacket.MessageType.PKT_SC_MOVE:
                        break;
                    case MyPacket.MessageType.PKT_SC_FEED:
                        {
                            MyPacket.Feed feed = deserializedProtoObject as MyPacket.Feed;
                            if (!mProductDict.ContainsKey(feed.ProductCode))
                            {
                                FeedClass fc = new FeedClass(feed.ProductCode);
                                mProductDict.Add(feed.ProductCode, fc);


                                this.Invoke(new Action(() =>
                                {
                                    mFeedBindingList.Add(fc);
                                    dataGridView1.DataSource = mFeedBindingList;
                                }));
                            }
                            else
                            {
                                FeedClass fc = mProductDict[feed.ProductCode];
                                fc.UpdateFields(feed);
                            }
                        }
                        break;

                    case MyPacket.MessageType.PKT_CS_SERVER_STATUS:
                    case MyPacket.MessageType.PKT_SC_SERVER_STATUS:
                        {
                            MyPacket.ServerStatus status = deserializedProtoObject as MyPacket.ServerStatus;

                            this.Invoke( new Action( () =>
                            {
                            textBox_count.Text = status.SessionCount.ToString();

                            if (status.SessionCount < 100)
                                textBox_count.BackColor = Color.Green;
                            else if (status.SessionCount < 500)
                                textBox_count.BackColor = Color.LightGreen;
                            else if (status.SessionCount < 1000)
                                textBox_count.BackColor = Color.LightCyan;
                            else if (status.SessionCount < 1500)
                                textBox_count.BackColor = Color.Blue;
                            else if (status.SessionCount < 2000)
                                textBox_count.BackColor = Color.Yellow;
                            else if (status.SessionCount < 3000)
                                textBox_count.BackColor = Color.Orange;
                            else if (status.SessionCount < 4000)
                                textBox_count.BackColor = Color.LightPink;
                            else if (status.SessionCount < 5000)
                                textBox_count.BackColor = Color.Red;
                            
                            }));

                            

                        }
                        break;
                    default:
                        break;
                }
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            MyPacket.ServerStatus status = new MyPacket.ServerStatus();
            status.SessionCount = 1;
            mSessionMgr.SendPacket(MyPacket.MessageType.PKT_CS_SERVER_STATUS, status);
        }
        

        BindingList<FeedClass> mFeedBindingList;
        Dictionary<string, FeedClass> mProductDict;
        SessionMgr mSessionMgr;


        
    }
}
