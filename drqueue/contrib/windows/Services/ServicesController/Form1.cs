using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using System.ServiceProcess;
using System.Diagnostics;

namespace ServicesController
{
	/// <summary>
	/// Description résumée de Form1.
	/// </summary>
	public class Form1 : System.Windows.Forms.Form
	{
		private System.Timers.Timer timer1;
		private System.Windows.Forms.Button button_ipc;
		private System.Windows.Forms.Button button_master;
		private System.Windows.Forms.Button button_slave;
		private System.ServiceProcess.ServiceController serviceControllerIpc;
		private System.ServiceProcess.ServiceController serviceControllerMaster;
		private System.ServiceProcess.ServiceController serviceControllerSlave;
		private System.Windows.Forms.StatusBar statusBar1;
		private System.Windows.Forms.NotifyIcon notifyIcon1;
		private System.Windows.Forms.Button button1;
		private System.Windows.Forms.Button button2;
		private System.ComponentModel.IContainer components;

		public Form1()
		{
			//
			// Requis pour la prise en charge du Concepteur Windows Forms
			//
			InitializeComponent();

			//
			// TODO : ajoutez le code du constructeur après l'appel à InitializeComponent
			//
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
		private void InitializeComponent()
		{
			this.components = new System.ComponentModel.Container();
			System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(Form1));
			this.timer1 = new System.Timers.Timer();
			this.button_ipc = new System.Windows.Forms.Button();
			this.button_master = new System.Windows.Forms.Button();
			this.button_slave = new System.Windows.Forms.Button();
			this.serviceControllerIpc = new System.ServiceProcess.ServiceController();
			this.serviceControllerMaster = new System.ServiceProcess.ServiceController();
			this.serviceControllerSlave = new System.ServiceProcess.ServiceController();
			this.statusBar1 = new System.Windows.Forms.StatusBar();
			this.notifyIcon1 = new System.Windows.Forms.NotifyIcon(this.components);
			this.button1 = new System.Windows.Forms.Button();
			this.button2 = new System.Windows.Forms.Button();
			((System.ComponentModel.ISupportInitialize)(this.timer1)).BeginInit();
			this.SuspendLayout();
			// 
			// timer1
			// 
			this.timer1.Enabled = true;
			this.timer1.SynchronizingObject = this;
			this.timer1.Elapsed += new System.Timers.ElapsedEventHandler(this.timer1_Elapsed);
			// 
			// button_ipc
			// 
			this.button_ipc.Location = new System.Drawing.Point(16, 8);
			this.button_ipc.Name = "button_ipc";
			this.button_ipc.Size = new System.Drawing.Size(248, 24);
			this.button_ipc.TabIndex = 2;
			this.button_ipc.Text = "Run ipc service";
			this.button_ipc.Click += new System.EventHandler(this.button_ipc_Click);
			// 
			// button_master
			// 
			this.button_master.Location = new System.Drawing.Point(16, 40);
			this.button_master.Name = "button_master";
			this.button_master.Size = new System.Drawing.Size(248, 24);
			this.button_master.TabIndex = 3;
			this.button_master.Text = "Run master service";
			this.button_master.Click += new System.EventHandler(this.button_master_Click);
			// 
			// button_slave
			// 
			this.button_slave.Location = new System.Drawing.Point(16, 72);
			this.button_slave.Name = "button_slave";
			this.button_slave.Size = new System.Drawing.Size(248, 24);
			this.button_slave.TabIndex = 4;
			this.button_slave.Text = "Run slave service";
			this.button_slave.Click += new System.EventHandler(this.button_slave_Click);
			// 
			// serviceControllerIpc
			// 
			this.serviceControllerIpc.ServiceName = "drqueue_ipc";
			// 
			// serviceControllerMaster
			// 
			this.serviceControllerMaster.ServiceName = "drqueue_master";
			// 
			// serviceControllerSlave
			// 
			this.serviceControllerSlave.ServiceName = "drqueue_slave";
			// 
			// statusBar1
			// 
			this.statusBar1.Location = new System.Drawing.Point(0, 141);
			this.statusBar1.Name = "statusBar1";
			this.statusBar1.Size = new System.Drawing.Size(280, 16);
			this.statusBar1.TabIndex = 5;
			// 
			// notifyIcon1
			// 
			this.notifyIcon1.Icon = ((System.Drawing.Icon)(resources.GetObject("notifyIcon1.Icon")));
			this.notifyIcon1.Text = "Drqueue";
			this.notifyIcon1.Visible = true;
			this.notifyIcon1.Click += new System.EventHandler(this.notifyIcon1_Click);
			// 
			// button1
			// 
			this.button1.Location = new System.Drawing.Point(184, 104);
			this.button1.Name = "button1";
			this.button1.Size = new System.Drawing.Size(48, 24);
			this.button1.TabIndex = 6;
			this.button1.Text = "Close";
			this.button1.Click += new System.EventHandler(this.button1_Click);
			// 
			// button2
			// 
			this.button2.Location = new System.Drawing.Point(40, 104);
			this.button2.Name = "button2";
			this.button2.Size = new System.Drawing.Size(88, 24);
			this.button2.TabIndex = 7;
			this.button2.Text = "run DRQMan";
			this.button2.Click += new System.EventHandler(this.button2_Click);
			// 
			// Form1
			// 
			this.AutoScale = false;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(280, 157);
			this.Controls.Add(this.button2);
			this.Controls.Add(this.button1);
			this.Controls.Add(this.statusBar1);
			this.Controls.Add(this.button_slave);
			this.Controls.Add(this.button_master);
			this.Controls.Add(this.button_ipc);
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "Form1";
			this.ShowInTaskbar = false;
			this.Text = "Drqueue Service Controller";
			this.WindowState = System.Windows.Forms.FormWindowState.Minimized;
			this.Load += new System.EventHandler(this.Form1_Load);
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

		private void timer1_Elapsed(object sender, System.Timers.ElapsedEventArgs e)
		{

			serviceControllerIpc.Refresh();
			serviceControllerMaster.Refresh();
			serviceControllerSlave.Refresh();
			if (serviceControllerIpc.Status == System.ServiceProcess.ServiceControllerStatus.Stopped)
				button_ipc.Text = "Run ipc service";
			else
				button_ipc.Text = "Stop ipc service";

			if (serviceControllerMaster.Status == System.ServiceProcess.ServiceControllerStatus.Stopped)
				button_master.Text = "Run master service";
			else
				button_master.Text = "Stop master service";

			if (serviceControllerSlave.Status == System.ServiceProcess.ServiceControllerStatus.Stopped)
				button_slave.Text = "Run slave service";
			else
				button_slave.Text = "Stop slave service";
		}

		private void button_ipc_Click(object sender, System.EventArgs e)
		{
			try
			{
				if (serviceControllerIpc.Status == System.ServiceProcess.ServiceControllerStatus.Stopped)
					serviceControllerIpc.Start();
				else
				{
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
				if (serviceControllerSlave.Status == System.ServiceProcess.ServiceControllerStatus.Stopped)
					serviceControllerSlave.Start();
				else
				{
					if (serviceControllerSlave.Status == System.ServiceProcess.ServiceControllerStatus.Running)
						serviceControllerSlave.Stop();		
				}
			}
			catch(System.Exception ex)
			{
				statusBar1.Text = ex.Message.ToString();
			}

		}

		private void Form1_Load(object sender, System.EventArgs e)
		{
			timer1.Start();
		}

		private void notifyIcon1_Click(object sender, System.EventArgs e)
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

		private void button1_Click(object sender, System.EventArgs e)
		{
			this.ShowInTaskbar = true;
			this.WindowState = FormWindowState.Minimized;
			this.ShowInTaskbar = false;
		}

		private void button2_Click(object sender, System.EventArgs e)
		{
			Process theProcess = new Process();
			theProcess.StartInfo.UseShellExecute = true;
			theProcess.StartInfo.FileName = "drqman.exe";
			theProcess.StartInfo.WorkingDirectory = Environment.GetEnvironmentVariable("DRQUEUE_BIN");
			theProcess.Start();
		}

	}
}
