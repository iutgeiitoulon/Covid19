using SciChart.Charting.Visuals;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using WpfRespirateur_Interface;

namespace Interface_Respirateur
{
    class Respirateur
    {
        static object ExitLock = new object();

        
        
        static WpfRespirateurInterface respirateurInterface;
        [STAThread] //à ajouter au projet initial
        static void Main(string[] args)
        {
            SciChartSurface.SetRuntimeLicenseKey(@"<LicenseContract>
              <Customer>University of  Toulon</Customer>
              <OrderId>EDUCATIONAL-USE-0109</OrderId>
              <LicenseCount>1</LicenseCount>
              <IsTrialLicense>false</IsTrialLicense>
              <SupportExpires>11/04/2019 00:00:00</SupportExpires>
              <ProductCode>SC-WPF-SDK-PRO-SITE</ProductCode>
              <KeyCode>lwABAQEAAABZVzOfQ0zVAQEAewBDdXN0b21lcj1Vbml2ZXJzaXR5IG9mICBUb3Vsb247T3JkZXJJZD1FRFVDQVRJT05BTC1VU0UtMDEwOTtTdWJzY3JpcHRpb25WYWxpZFRvPTA0LU5vdi0yMDE5O1Byb2R1Y3RDb2RlPVNDLVdQRi1TREstUFJPLVNJVEWDf0QgB8GnCQXI6yAqNM2njjnGbUt2KsujTDzeE+k69K1XYVF1s1x1Hb/i/E3GHaU=</KeyCode>
            </LicenseContract>");
            StartRobotInterface();
            lock (ExitLock)
            {
                // Do whatever setup code you need here
                // once we are done wait
                System.Threading.Monitor.Wait(ExitLock);
            }
        }

        static Thread t1;
        static void StartRobotInterface()
        {
            t1 = new Thread(() =>
            {
                //Attention, il est nécessaire d'ajouter PresentationFramework, PresentationCore, WindowBase and your wpf window application aux ressources.
                respirateurInterface = new WpfRespirateurInterface();
                //respirateurInterface.Loaded += RegisterRobotInterfaceEvents;
                respirateurInterface.ShowDialog();

            });
            t1.SetApartmentState(ApartmentState.STA);
            t1.Start();
        }
    }
}
