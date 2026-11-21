USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetRestraintConstant]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-05-20
description : ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetRestraintConstant]
	  @intPlayRestraint int OUTPUT
	, @intResetRestraint int OUTPUT
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;

SELECT @intPlayRestraint = PlayRestraint, @intResetRestraint = ResetRestraint
FROM dbo.AbuseMonitorRestraintConstants WITH (NOLOCK);

RETURN 0;
GO
