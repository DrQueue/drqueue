using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using System.ServiceProcess;
using System.Diagnostics;
using System.Resources;
using System.Reflection;

namespace ServicesController
{
	/// <summary>
	/// Description résumée de Form1.
	/// </summary>
	public class Form1 : System.Windows.Forms.Form
	{
		private System.ComponentModel.IContainer components;
		private System.Windows.Forms.Button button_master;
		private System.Windows.Forms.Button button2;
		private System.Windows.Forms.Button button1;
		private System.Windows.Forms.NotifyIcon DrQueue;
		private System.Windows.Forms.Button button_ipc;
		private System.Windows.Forms.Button button3;
		private System.Timers.Timer timer1;
		private System.Windows.Forms.Button button_slave;
		private System.Windows.Forms.Label statusBar1;

		private System.ServiceProcess.ServiceController serviceControllerIpc;
		private System.ServiceProcess.ServiceController serviceControllerMaster;

		private const string IPC = "ipc-daemon2";
		private const string MASTER = "master";
		private const string SLAVE = "slave";

		public Form1()
		{
			//
			// Requis pour la prise en charge du Concepteur Windows Forms
			//
			InitializeComponent();

			this.serviceControllerIpc = new System.ServiceProcess.ServiceController();
			this.serviceControllerMaster = new System.ServiceProcess.ServiceController();
			// 
			// serviceControllerIpc
			// 
			this.serviceControllerIpc.ServiceName = "drqueue_ipc";
			// 
			// serviceControllerMaster
			// 
			this.serviceControllerMaster.ServiceName = "drqueue_master";
		}

		/// <summary>
		/// Nettoyage des ressources utilisées.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if (components != null) 
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Code généré par le Concepteur Windows Form
		/// <summary>
		/// Méthode requise pour la prise en charge du concepteur - ne modifiez pas
		/// le contenu de cette méthode avec l'éditeur de code.
		/// </summary>
		private void InitializeComponent() {
			this.components = new System.ComponentModel.Container();
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(Form1));
			this.statusBar1 = new System.Windows.Forms.Label();
			this.button_slave = new System.Windows.Forms.Button();
			this.timer1 = new System.Timers.Timer();
			this.button3 = new System.Windows.Forms.Button();
			this.button_ipc = new System.Windows.Forms.Button();
			this.DrQueue = new System.Windows.Forms.NotifyIcon(this.components);
			this.button1 = new System.Windows.Forms.Button();
			this.button2 = new System.Windows.Forms.Button();
			this.button_master = new System.Windows.Forms.Button();
			((System.ComponentModel.ISupportInitialize)(this.timer1)).BeginInit();
			this.SuspendLayout();
			// 
			// statusBar1
			// 
			this.statusBar1.Location = new System.Drawing.Point(8, 128);
			this.statusBar1.Name = "statusBar1";
			this.statusBar1.Size = new System.Drawing.Size(272, 16);
			this.statusBar1.TabIndex = 8;
			// 
			// button_slave
			// 
			this.button_slave.BackColor = System.Drawing.Color.DimGray;
			this.button_slave.Location = new System.Drawing.Point(160, 24);
			this.button_slave.Name = "button_slave";
			this.button_slave.Size = new System.Drawing.Size(112, 24);
			this.button_slave.TabIndex = 4;
			this.button_slave.Text = "CONNECTED";
			this.button_slave.Click += new System.EventHandler(this.button_slave_Click);
			// 
			// timer1
			// 
			this.timer1.Enabled = true;
			this.timer1.Interval = 500;
			this.timer1.SynchronizingObject = this;
			this.timer1.Elapsed += new System.Timers.ElapsedEventHandler(this.timer1_Elapsed);
			// 
			// button3
			// 
			this.button3.BackColor = System.Drawing.Color.Gray;
			this.button3.Location = new System.Drawing.Point(16, 96);
			this.button3.Name = "button3";
			this.button3.Size = new System.Drawing.Size(72, 24);
			this.button3.TabIndex = 9;
			this.button3.Text = "Quit";
			this.button3.Click += new System.EventHandler(this.button3_Click);
			// 
			// button_ipc
			// 
			this.button_ipc.BackColor = System.Drawing.Color.DimGray;
			this.button_ipc.Location = new System.Drawing.Point(16, 24);
			this.button_ipc.Name = "button_ipc";
			this.button_ipc.Size = new System.Drawing.Size(72, 24);
			this.button_ipc.TabIndex = 2;
			this.button_ipc.Text = "DRIVERS";
			this.button_ipc.Click += new System.EventHandler(this.button_ipc_Click);
			// 
			// DrQueue
			// 
			this.DrQueue.Icon = ((System.Drawing.Icon)(resources.GetObject("DrQueue.Icon")));
			this.DrQueue.Text = "DrQeue";
			this.DrQueue.Visible = true;
			this.DrQueue.MouseDown += new System.Windows.Forms.MouseEventHandler(this.NotifyIcon1MouseDown);
			// 
			// button1
			// 
			this.button1.BackColor = System.Drawing.Color.Gray;
			this.button1.Location = new System.Drawing.Point(88, 96);
			this.button1.Name = "button1";
			this.button1.Size = new System.Drawing.Size(184, 24);
			this.button1.TabIndex = 6;
			this.button1.Text = "Close window";
			this.button1.Click += new System.EventHandler(this.button1_Click);
			// 
			// button2
			// 
			this.button2.BackColor = System.Drawing.Color.Gray;
			this.button2.Location = new System.Drawing.Point(16, 56);
			this.button2.Name = "button2";
			this.button2.Size = new System.Drawing.Size(256, 32);
			this.button2.TabIndex = 7;
			this.button2.Text = "Manage jobs";
			this.button2.Click += new System.EventHandler(this.Button2Click);
			// 
			// button_master
			// 
			this.button_master.BackColor = System.Drawing.Color.DimGray;
			this.button_master.Location = new System.Drawing.Point(88, 24);
			this.button_master.Name = "button_master";
			this.button_master.Size = new System.Drawing.Size(72, 24);
			this.button_master.TabIndex = 3;
			this.button_master.Text = "MASTER";
			this.button_master.Click += new System.EventHandler(this.button_master_Click);
			// 
			// Form1
			// 
			this.AutoScale = false;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.BackColor = System.Drawing.Color.Black;
			this.ClientSize = new System.Drawing.Size(288, 149);
			this.Controls.Add(this.button3);
			this.Controls.Add(this.statusBar1);
			this.Controls.Add(this.button2);
			this.Controls.Add(this.button1);
			this.Controls.Add(this.button_slave);
			this.Controls.Add(this.button_master);
			this.Controls.Add(this.button_ipc);
			this.ForeColor = System.Drawing.Color.White;
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "Form1";
			this.ShowInTaskbar = false;
			this.Text = "Drqueue Service Controller";
			this.WindowState = System.Windows.Forms.FormWindowState.Minimized;
			this.Load += new System.EventHandler(this.Form1_Load);
			this.Closed += new System.EventHandler(this.Form1Closed);
			((System.ComponentModel.ISupportInitialize)(this.timer1)).EndInit();
			this.ResumeLayout(false);
		}
		#endregion


		/// <summary>
		/// Point d'entrée principal de l'application.
		/// </summary>
		[STAThread]
		static void Main() 
		{
			Application.Run(new Form1());
		}

		
		private bool IsActive(string name)
		{
			Process []findProcess = Process.GetProcessesByName(name);
			return (findProcess.Length > 0);
		}

		private void Activate(string name)
		{
			Process theProcess = new Process();
			theProcess.StartInfo.UseShellExecute = true;
			theProcess.StartInfo.WorkingDirectory = Environment.GetEnvironmentVariable("DRQUEUE_BIN");
			theProcess.StartInfo.FileName = name + ".exe";
			theProcess.Start();			
		}

		private void Kill(string name)
		{
				Process []findProcess = Process.GetProcessesByName(name);
				foreach (Process proc in findProcess)
					proc.Kill();
				System.Threading.Thread.Sleep(1000);
		}

		private void timer1_Elapsed(object sender, System.Timers.ElapsedEventArgs e)
		{
			serviceControllerIpc.Refresh();
			serviceControllerMaster.Refresh();
			
			if (serviceControllerIpc.Status == System.ServiceProcess.ServiceControllerStatus.Stopped)
				button_ipc.BackColor = System.Drawing.Color.DimGray;
			else
				button_ipc.BackColor = System.Drawing.Color.Red;

			if (serviceControllerMaster.Status == System.ServiceProcess.ServiceControllerStatus.Stopped)
				button_master.BackColor = System.Drawing.Color.DimGray;
			else
				button_master.BackColor = System.Drawing.Color.Red;

			if (!IsActive(SLAVE))
				button_slave.BackColor = System.Drawing.Color.DimGray;
			else
				button_slave.BackColor = System.Drawing.Color.Red;
		}

		private void button_ipc_Click(object sender, System.EventArgs e)
		{
			try
			{
				if (serviceControllerIpc.Status == System.ServiceProcess.ServiceControllerStatus.Stopped)
					serviceControllerIpc.Start();
				else
				{
					if (IsActive(SLAVE))
						Kill(SLAVE);										
					if (serviceControllerIpc.Status == System.ServiceProcess.ServiceControllerStatus.Running)
						serviceControllerIpc.Stop();
				}
			}
			catch(System.Exception ex)
			{
				statusBar1.Text = ex.Message.ToString();
			}
		}

		private void button_master_Click(object sender, System.EventArgs e)
		{
			try
			{
				if (serviceControllerMaster.Status == System.ServiceProcess.ServiceControllerStatus.Stopped)
					serviceControllerMaster.Start();
				else
				{
					if (serviceControllerMaster.Status == System.ServiceProcess.ServiceControllerStatus.Running)
						serviceControllerMaster.Stop();		
				}
			}
			catch(System.Exception ex)
			{
				statusBar1.Text = ex.Message.ToString();
			}
		}

		private void button_slave_Click(object sender, System.EventArgs e)
		{
			try
			{
				if (!IsActive(SLAVE))
				{
					if (serviceControllerIpc.Status == System.ServiceProcess.ServiceControllerStatus.Stopped)
					{
						serviceControllerIpc.Start();
						System.Threading.Thread.Sleep(1000);
					}
					Activate(SLAVE);
				}
				else
					Kill(SLAVE);
			}
			catch(System.Exception ex)
			{
				statusBar1.Text = ex.Message.ToString();
			}
		}

		private void Form1_Load(object sender, System.EventArgs e)
		{
			string MasterName = Environment.GetEnvironmentVariable("DRQUEUE_MASTER");
			string ComputerName = Environment.GetEnvironmentVariable("COMPUTERNAME");
			string IsSlave = Environment.GetEnvironmentVariable("DRQUEUE_ISSLAVE");

			if ((IsSlave == "1") && !IsActive(SLAVE))
			{
				try
				{
					if (serviceControllerIpc.Status == System.ServiceProcess.ServiceControllerStatus.Stopped)
					{
						serviceControllerIpc.Start();
						System.Threading.Thread.Sleep(1000);
					}
				}
				catch(System.Exception ex)
				{
					statusBar1.Text = ex.Message;
				}
				Activate(SLAVE);
			}
			timer1.Start();
		}

		private void button1_Click(object sender, System.EventArgs e)
		{
			this.ShowInTaskbar = true;
			this.WindowState = FormWindowState.Minimized;
			this.ShowInTaskbar = false;
		}

		void NotifyIcon1MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			if (this.WindowState == FormWindowState.Normal)
			{
				this.ShowInTaskbar = true;
				this.WindowState = FormWindowState.Minimized;
				this.ShowInTaskbar = false;
			}
			else
				this.WindowState = FormWindowState.Normal;
		}
		
		void button3_Click(object sender, System.EventArgs e)
		{
			
			this.Close();
		}
		
		void Button2Click(object sender, System.EventArgs e)
		{
			Activate("drqman");
		}
		
		void Form1Closed(object sender, System.EventArgs e)
		{
			Kill(SLAVE);
		}
	}
}
