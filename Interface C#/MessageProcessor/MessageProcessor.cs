using Constants;
using EventArgsLibrary;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Timers;
using Utilities;

namespace MessageProcessor
{
    public class MsgProcessor
    {
        Timer tmrComptageMessage;
        public MsgProcessor()
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
        public void ProcessDecodedMessage(object sender, MessageDecodedArgs e)
        {
            ProcessDecodedMessage((Int16)e.MsgFunction,(Int16) e.MsgPayloadLength, e.MsgPayload);
        }
        //Processeur de message en provenance du respirateur...
        //Une fois processé, le message sera transformé en event sortant
        public void ProcessDecodedMessage(Int16 command, Int16 payloadLength, byte[] payload)
        {
            byte[] tab;
            uint timeStamp;
            switch (command)
            {


                case (short)Commands.PressureDataFromRespirator:
                    {
                        uint time2 = (uint)(payload[3] | payload[2] << 8 | payload[1] << 16 | payload[0] << 24);
                        byte[] tab2 = payload.GetRange(4, 4);
                        float sensor1Pressure = tab2.GetFloat();
                        tab2 = payload.GetRange(8, 4);
                        float sensor2Pressure = tab2.GetFloat();
                        tab2 = payload.GetRange(12, 4);
                        float sensorAmbiantPressure = tab2.GetFloat();
                        //On envois l'event aux abonnés
                        OnPressureDataFromRespirator(time2, sensor1Pressure, sensor2Pressure, sensorAmbiantPressure);
                    }
                    break;



                case (short)Commands.ErrorTextMessage:
                    string errorMsg = Encoding.UTF8.GetString(payload);
                    //On envois l'event aux abonnés
                    OnErrorTextFromRespirateur(errorMsg);
                    break;
                default: break;
            }
        }


        public event EventHandler<StringEventArgs> OnErrorTextFromRespirateurGeneratedEvent;
        public virtual void OnErrorTextFromRespirateur(string str)
        {
            var handler = OnErrorTextFromRespirateurGeneratedEvent;
            if (handler != null)
            {
                handler(this, new StringEventArgs { value = str });
            }
        }

        //Output events
        public event EventHandler<RespirateurDataEventArgs> OnPressureDataFromRespiratorGeneratedEvent;
        public virtual void OnPressureDataFromRespirator(uint timeStamp, double pressureSensor1, double pressureSensor2, double pressureSensorAmbiant)
        {
            var handler = OnPressureDataFromRespiratorGeneratedEvent;
            if (handler != null)
            {
                handler(this, new RespirateurDataEventArgs { EmbeddedTimeStampInMs = timeStamp, pressureSensor1 = pressureSensor1, pressureSensor2 = pressureSensor2, pressureSensorAmbiant= pressureSensorAmbiant });
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
