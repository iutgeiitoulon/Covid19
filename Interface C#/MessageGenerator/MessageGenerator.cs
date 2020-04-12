using System;
using EventArgsLibrary;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Constants;

namespace MessageGenerator
{
    public class MsgGenerator
    {
        //Input events
        public void GenerateMessageSTART(object sender, BoolEventArgs e)
        {
            byte[] payload = new byte[1];
            payload[0] = Convert.ToByte(e.value);
            OnMessageToRespirator((Int16)Commands.START, 1, payload);
        }

        public void GenerateMessageSTOP(object sender, BoolEventArgs e)
        {
            byte[] payload = new byte[1];
            payload[0] = Convert.ToByte(e.value);

            OnMessageToRespirator((Int16)Commands.EmergencySTOP, 1, payload);
        }


        //Output events
        public event EventHandler<MessageToRobotArgs> OnMessageToRobotGeneratedEvent;
        public virtual void OnMessageToRespirator(Int16 msgFunction, Int16 msgPayloadLength, byte[] msgPayload)
        {
            var handler = OnMessageToRobotGeneratedEvent;
            if (handler != null)
            {
                handler(this, new MessageToRobotArgs { MsgFunction = msgFunction, MsgPayloadLength = msgPayloadLength, MsgPayload=msgPayload});
            }
        }
    }
}
