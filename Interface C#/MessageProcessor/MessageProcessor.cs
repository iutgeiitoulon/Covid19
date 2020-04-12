using Constants;
using EventArgsLibrary;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Timers;
using Utilities;

namespace RobotMessageProcessor
{
    public class RobotMsgProcessor
    {
        Timer tmrComptageMessage;
        public RobotMsgProcessor()
        {
            tmrComptageMessage = new Timer(1000);
            tmrComptageMessage.Elapsed += TmrComptageMessage_Elapsed;
            tmrComptageMessage.Start();
        }

        int nbMessageIMUReceived = 0;
        int nbMessageSpeedReceived = 0;
        private void TmrComptageMessage_Elapsed(object sender, ElapsedEventArgs e)
        {
            OnMessageCounter(nbMessageIMUReceived, nbMessageSpeedReceived);
            nbMessageIMUReceived = 0;
            nbMessageSpeedReceived = 0;
        }

        //Input CallBack        
        public void ProcessRobotDecodedMessage(object sender, MessageDecodedArgs e)
        {
            ProcessDecodedMessage((Int16)e.MsgFunction,(Int16) e.MsgPayloadLength, e.MsgPayload);
        }
        //Processeur de message en provenance du robot...
        //Une fois processé, le message sera transformé en event sortant
        public void ProcessDecodedMessage(Int16 command, Int16 payloadLength, byte[] payload)
        {
            byte[] tab;
            uint timeStamp;
            switch (command)
            {


                case (short)Commands.MotorCurrents:
                    {
                        uint time2 = (uint)(payload[3] | payload[2] << 8 | payload[1] << 16 | payload[0] << 24);
                        byte[] tab2 = payload.GetRange(4, 4);
                        float motor1Current = tab2.GetFloat();
                        tab2 = payload.GetRange(8, 4);
                        float motor2Current = tab2.GetFloat();
                        tab2 = payload.GetRange(12, 4);
                        float motor3Current = tab2.GetFloat();
                        tab2 = payload.GetRange(16, 4);
                        float motor4Current = tab2.GetFloat();
                        tab2 = payload.GetRange(20, 4);
                        float motor5Current = tab2.GetFloat();
                        tab2 = payload.GetRange(24, 4);
                        float motor6Current = tab2.GetFloat();
                        tab2 = payload.GetRange(28, 4);
                        float motor7Current = tab2.GetFloat();
                        //On envois l'event aux abonnés
                        OnMotorsCurrentsFromRobot(time2, motor1Current, motor2Current, motor3Current, motor4Current, motor5Current, motor6Current, motor7Current);
                    }
                    break;



                case (short)Commands.ErrorTextMessage:
                    string errorMsg = Encoding.UTF8.GetString(payload);
                    //On envois l'event aux abonnés
                    OnErrorTextFromRobot(errorMsg);
                    break;
                default: break;
            }
        }

        public event EventHandler<EventArgs> OnWelcomeMessageFromRobotGeneratedEvent;
        public virtual void OnWelcomeMessageFromRobot()
        {
            var handler = OnWelcomeMessageFromRobotGeneratedEvent;
            if (handler != null)
            {
                handler(this, new EventArgs());
            }
        }


        //Output events
        public event EventHandler<IMUDataEventArgs> OnIMURawDataFromRobotGeneratedEvent;
        public virtual void OnIMUDataFromRobot(uint timeStamp, Point3D accelxyz, Point3D gyroxyz)
        {
            var handler = OnIMURawDataFromRobotGeneratedEvent;
            if (handler != null)
            {
                handler(this, new IMUDataEventArgs { EmbeddedTimeStampInMs = timeStamp, accelX = accelxyz.X, accelY = accelxyz.Y, accelZ= accelxyz.Z , gyroX=gyroxyz.X, gyroY=gyroxyz.Y, gyroZ=gyroxyz.Z });
            }
        }


        
        public event EventHandler<MsgCounterArgs> OnMessageCounterEvent;
        public virtual void OnMessageCounter(int nbMessageFromImu, int nbMessageFromOdometry)
        {
            var handler = OnMessageCounterEvent;
            if (handler != null)
            {
                handler(this, new MsgCounterArgs { nbMessageIMU = nbMessageFromImu, nbMessageOdometry = nbMessageFromOdometry });
            }
        }
    }
}
