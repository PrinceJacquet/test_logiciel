void oracle_sqrt(
  int Pre_a, int a, 
  int pathcrawler__retres__sqrt)
{
  int i = pathcrawler__retres__sqrt;

  /* A remplacer par un vrai verdict */
	if (a>=0)
	{

		if ( i * i <= a && a < (i +1) * (i +1))
		{
		   pathcrawler_verdict_success();
		}
		else 
		{
		   pathcrawler_verdict_failure();
		}
	}
	
	else 
	{
		if (i != 0)
		{
		   pathcrawler_verdict_failure();
		}
		else 
		{
		   pathcrawler_verdict_success();
		}
	}

  //pathcrawler_verdict_unknown();
  
  return;
  
}
