Imports System.ServiceProcess

Public Class Service1
    Inherits System.ServiceProcess.ServiceBase

#Region " Code généré par le Concepteur de composants "

    Public Sub New()
        MyBase.New()

        ' Cet appel est requis par le Concepteur de composants.
        InitializeComponent()

        ' Ajoutez une initialisation quelconque après l'appel InitializeComponent()

    End Sub

    'La méthode substituée Dispose du UserService pour nettoyer la liste des composants.
    Protected Overloads Overrides Sub Dispose(ByVal disposing As Boolean)
        If disposing Then
            If Not (components Is Nothing) Then
                components.Dispose()
            End If
        End If
        MyBase.Dispose(disposing)
    End Sub

    ' Le point d'entrée principal pour le processus
    <MTAThread()> _
    Shared Sub Main()
        Dim ServicesToRun() As System.ServiceProcess.ServiceBase

        ' Plusieurs services NT s'exécutent dans le même processus. Pour ajouter
        ' un autre service à ce processus, modifiez la ligne suivante
        ' pour créer un second objet service. Par exemple,
        '
        '   ServicesToRun = New System.ServiceProcess.ServiceBase () {New Service1, New MySecondUserService}
        '
        ServicesToRun = New System.ServiceProcess.ServiceBase () {New Service1}

        System.ServiceProcess.ServiceBase.Run(ServicesToRun)
    End Sub

    'Requis par le Concepteur de composants
    Private components As System.ComponentModel.IContainer

    ' REMARQUE : la procédure suivante est requise par le Concepteur de composants
    ' Elle peut être modifiée à l'aide du Concepteur de composants.  
    ' Ne la modifiez pas en utilisant l'éditeur de code.
    <System.Diagnostics.DebuggerStepThrough()> Private Sub InitializeComponent()
        '
        'Service1
        '
        Me.CanShutdown = True
        Me.ServiceName = "Service1"

    End Sub

#End Region


    Dim theProcess As New Process

    Protected Overrides Sub OnStart(ByVal args() As String)
        ' Ajoutez ici le code pour démarrer votre service. Cette méthode doit permettre à votre
        ' service de faire son travail.

        theProcess.StartInfo.UseShellExecute = True
        theProcess.StartInfo.FileName = "ipc-daemon2.exe"
        theProcess.StartInfo.WorkingDirectory = Environment.GetEnvironmentVariable("DRQUEUE_BIN")
        theProcess.Start()
    End Sub

    Protected Overrides Sub OnStop()
        ' Ajoutez ici le code pour effectuer les destructions nécessaires à l'arrêt de votre service.

        ' Dim findProcess As Process() = Process.GetProcessesByName("ipc-daemon2")

        ' For Each proc As Process In findProcess
        ' proc.Kill()
        ' Next

        'theProcess.StartInfo.UseShellExecute = True
        'theProcess.StartInfo.FileName = "pskill.exe"
        'theProcess.StartInfo.Arguments = "ipc-daemon2.exe"
        'theProcess.StartInfo.WorkingDirectory = Environment.GetEnvironmentVariable("DRQUEUE_BIN")
        'theProcess.Start()

        'Dim shell
        'shell = CreateObject("wscript.shell")
        'shell.run("%DRQUEUE_ROOT%\bin\pskill.exe ipc-daemon2.exe")
        'shell = Nothing

    End Sub

End Class
