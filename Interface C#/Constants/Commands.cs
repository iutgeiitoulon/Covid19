using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Constants
{
    public enum Commands
    {
        #region Commandes générales
        #pragma warning disable CS1591

        Unknown = 0,
        START=1,

        GoToXYTheta = 1000,

        EmergencySTOP=-1,


#pragma warning restore CS1591
        #endregion
    }

    public enum MotorControlName
    {
#pragma warning disable CS1591
        MotorLeft,
        MotorRear,
        MotorRight,
        Motor4,
        Motor5,
        Motor6,
        None
#pragma warning restore CS1591
    }
}
