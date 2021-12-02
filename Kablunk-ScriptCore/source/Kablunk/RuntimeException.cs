using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Kablunk
{
	public class RuntimeException
	{

		public static void OnException(object exception)
		{
			Log.Error("RuntimeException.OnException");
			if (exception != null)
			{
				if (exception is NullReferenceException)
				{
					var e = exception as NullReferenceException;
					Console.WriteLine(e.Message);
					Console.WriteLine(e.Source);
					Console.WriteLine(e.StackTrace);
				}
				else if (exception is Exception)
				{
					var e = exception as Exception;
					Console.WriteLine(e.Message);
					Console.WriteLine(e.Source);
					Console.WriteLine(e.StackTrace);
				}
			}
			else
			{
				Log.Error("Exception is null");
			}

		}

	}

}
