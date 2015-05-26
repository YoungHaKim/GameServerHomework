namespace ServerGUI
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.button_connect = new System.Windows.Forms.Button();
            this.textBox_ip = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.numericUpDown_port = new System.Windows.Forms.NumericUpDown();
            this.dataGridView1 = new System.Windows.Forms.DataGridView();
            this.label3 = new System.Windows.Forms.Label();
            this.textBox_count = new System.Windows.Forms.TextBox();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown_port)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridView1)).BeginInit();
            this.SuspendLayout();
            // 
            // button_connect
            // 
            this.button_connect.Location = new System.Drawing.Point(13, 13);
            this.button_connect.Name = "button_connect";
            this.button_connect.Size = new System.Drawing.Size(150, 23);
            this.button_connect.TabIndex = 0;
            this.button_connect.Text = "Connect to Server";
            this.button_connect.UseVisualStyleBackColor = true;
            this.button_connect.Click += new System.EventHandler(this.button_connect_Click);
            // 
            // textBox_ip
            // 
            this.textBox_ip.Location = new System.Drawing.Point(200, 13);
            this.textBox_ip.Name = "textBox_ip";
            this.textBox_ip.Size = new System.Drawing.Size(100, 21);
            this.textBox_ip.TabIndex = 1;
            this.textBox_ip.Text = "127.0.0.1";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(175, 18);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(19, 12);
            this.label1.TabIndex = 2;
            this.label1.Text = "ip:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(302, 18);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(30, 12);
            this.label2.TabIndex = 2;
            this.label2.Text = "port:";
            // 
            // numericUpDown_port
            // 
            this.numericUpDown_port.Location = new System.Drawing.Point(339, 14);
            this.numericUpDown_port.Maximum = new decimal(new int[] {
            65536,
            0,
            0,
            0});
            this.numericUpDown_port.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.numericUpDown_port.Name = "numericUpDown_port";
            this.numericUpDown_port.Size = new System.Drawing.Size(77, 21);
            this.numericUpDown_port.TabIndex = 3;
            this.numericUpDown_port.Value = new decimal(new int[] {
            9990,
            0,
            0,
            0});
            // 
            // dataGridView1
            // 
            this.dataGridView1.AllowUserToAddRows = false;
            this.dataGridView1.AllowUserToDeleteRows = false;
            this.dataGridView1.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dataGridView1.Location = new System.Drawing.Point(13, 65);
            this.dataGridView1.Name = "dataGridView1";
            this.dataGridView1.ReadOnly = true;
            this.dataGridView1.RowTemplate.Height = 23;
            this.dataGridView1.Size = new System.Drawing.Size(930, 424);
            this.dataGridView1.TabIndex = 4;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(470, 21);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(132, 12);
            this.label3.TabIndex = 5;
            this.label3.Text = "Server Session Count:";
            // 
            // textBox_count
            // 
            this.textBox_count.BackColor = System.Drawing.Color.White;
            this.textBox_count.Font = new System.Drawing.Font("Gulim", 14.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(129)));
            this.textBox_count.Location = new System.Drawing.Point(609, 13);
            this.textBox_count.Name = "textBox_count";
            this.textBox_count.ReadOnly = true;
            this.textBox_count.Size = new System.Drawing.Size(100, 29);
            this.textBox_count.TabIndex = 6;
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(955, 501);
            this.Controls.Add(this.textBox_count);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.dataGridView1);
            this.Controls.Add(this.numericUpDown_port);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.textBox_ip);
            this.Controls.Add(this.button_connect);
            this.Name = "Form1";
            this.Text = "Form1";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            this.Load += new System.EventHandler(this.Form1_Load);
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown_port)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridView1)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button button_connect;
        private System.Windows.Forms.TextBox textBox_ip;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.NumericUpDown numericUpDown_port;
        private System.Windows.Forms.DataGridView dataGridView1;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox textBox_count;
    }
}

