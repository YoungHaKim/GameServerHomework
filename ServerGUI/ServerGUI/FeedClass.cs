using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ServerGUI
{
    public class DepthClass
    {
        public int mCount { get; set; }
        public int mQty { get; set; }
        public double mPrice { get; set; }
    }
    
    public class FeedClass : INotifyPropertyChanged
    {
        public FeedClass(string name)
        {
            this.Name = name;

            for (int i = 0; i < DEPTH_LEVELS; i++)
            {
                BidDepth[i] = new DepthClass();
                AskDepth[i] = new DepthClass();
            }

            if (PropertyChanged != null)
            {
                this.PropertyChanged(this, new PropertyChangedEventArgs("Name"));
            }
        }

        internal void UpdateFields(MyPacket.Feed feed)
        {
            BidDepth[0].mCount = feed.BidDepth[0].Count;
            BidDepth[0].mQty = feed.BidDepth[0].Qty;
            BidDepth[0].mPrice = feed.BidDepth[0].Price;


            AskDepth[0].mCount = feed.AskDepth[0].Count;
            AskDepth[0].mQty = feed.AskDepth[0].Qty;
            AskDepth[0].mPrice = feed.AskDepth[0].Price;

            this.PropertyChanged(this, new PropertyChangedEventArgs("BCnt"));
            this.PropertyChanged(this, new PropertyChangedEventArgs("BQty"));
            this.PropertyChanged(this, new PropertyChangedEventArgs("BPrc"));


            this.PropertyChanged(this, new PropertyChangedEventArgs("ACnt"));
            this.PropertyChanged(this, new PropertyChangedEventArgs("AQty"));
            this.PropertyChanged(this, new PropertyChangedEventArgs("APrc"));


        }

        // Columns appear in order declared here
        public string Name { get; set; }

        public int BCnt { get { return BidDepth[0].mCount; } }
        public int BQty { get { return BidDepth[0].mQty; } }
        public double BPrc { get { return BidDepth[0].mPrice; } }

        public double APrc { get { return AskDepth[0].mPrice; } }
        public int AQty { get { return AskDepth[0].mQty; } }
        public int ACnt { get { return AskDepth[0].mCount; } }

        public DepthClass[] BidDepth = new DepthClass[DEPTH_LEVELS];
        public DepthClass[] AskDepth = new DepthClass[DEPTH_LEVELS];


        private const int DEPTH_LEVELS = 10;

        public event PropertyChangedEventHandler PropertyChanged;

    }
}
