using System;
using System.Collections;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.ServiceProcess;

namespace Services
{
	public class Service_ipc : System.ServiceProcess.ServiceBase
	{
		private System.Timers.Timer timer1;
		private System.Diagnostics.EventLog eventLog1;
		/// <summary> 
		/// Variable nécessaire au concepteur.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public Service_ipc()
		{
			// Cet appel est requis par le Concepteur de composants Windows.Forms.
			InitializeComponent();

			// TODO : ajoutez les initialisations après l'appel à InitComponent
		}

		// Le point d'entrée principal pour le processus
		static void Main()
		{
			System.ServiceProcess.ServiceBase[] ServicesToRun;
	
			// Plusieurs services utilisateurs peuvent s'exécuter dans le même processus. Pour ajouter
			// un autre service à ce processus, modifiez la ligne suivante
			// afin de créer un second objet service. Par exemple,
			//
			//   ServicesToRun = New System.ServiceProcess.ServiceBase[] {new Service1(), new MySecondUserService()};
			//
			ServicesToRun = new System.ServiceProcess.ServiceBase[] { new Service_ipc() };

			System.ServiceProcess.ServiceBase.Run(ServicesToRun);
		}

		/// <summary> 
		/// Méthode requise pour la prise en charge du concepteur - ne modifiez pas 
		/// le contenu de cette méthode avec l'éditeur de code.
		/// </summary>
		private void InitializeComponent()
		{
			this.timer1 = new System.Timers.Timer();
			this.eventLog1 = new System.Diagnostics.EventLog();
			((System.ComponentModel.ISupportInitialize)(this.timer1)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.eventLog1)).BeginInit();
			// 
			// timer1
			// 
			this.timer1.Enabled = true;
			this.timer1.Interval = 1000;
			this.timer1.Elapsed += new System.Timers.ElapsedEventHandler(this.timer1_Elapsed);
			// 
			// eventLog1
			// 
			this.eventLog1.Log = "Application";
			this.eventLog1.Source = "ipc";
			// 
			// Service_ipc
			// 
			this.ServiceName = "Service_ipc";
			((System.ComponentModel.ISupportInitialize)(this.timer1)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.eventLog1)).EndInit();

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

		/// <summary>
		/// Démarrage du service.
		/// </summary>
		/// 
		protected override void OnStart(string[] args)
		{
			//eventLog1.WriteEntry("started !");

			Process theProcess = new Process();
			theProcess.StartInfo.UseShellExecute = true;
			theProcess.StartInfo.FileName = "ipc-daemon2.exe";
	        theProcess.StartInfo.WorkingDirectory = Environment.GetEnvironmentVariable("DRQUEUE_BIN");
		    theProcess.Start();
		    //try
			//{
		    	//Process []findProcess;
		    	//do
		    	//{
		    		//findProcess = Process.GetProcessesByName("ipc-daemon2");
		    	//}
		    	//while (findProcess.Length < 1);
				timer1.Start();
		    //}
			//catch (System.ServiceProcess.TimeoutException)
			//{
			//}
		}
 
		/// <summary>
		/// Arrêt du service.
		/// </summary>
		protected override void OnStop()
		{
			try
			{
				Process []findProcess = Process.GetProcessesByName("ipc-daemon2");
				foreach (Process proc in findProcess)
				{
					proc.Kill();
				}
				timer1.Stop();
			}
			catch (System.ServiceProcess.TimeoutException)
			{
				timer1.Stop();
			}
		}

		private void timer1_Elapsed(object sender, System.Timers.ElapsedEventArgs e)
		{
			Process []findProcess = Process.GetProcessesByName("ipc-daemon2");
			if (findProcess.Length < 1)
			{
				ServiceController ctrl = new ServiceController("drqueue_ipc");
				ctrl.Stop();
			}
		}
	}
}
