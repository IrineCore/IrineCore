USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetLeavePreventionConnInfo]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetLeavePreventionConnInfo]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetLeavePreventionConnInfo] AS' 
END
GO
/**
version : 12
author : ???
e-mail : wgkim@eyedentitygames
created date : 2010-12-23
description : ???? ??? ?? ?? ??? ?????.
result set : 
  @intAccountID				, @dtmLastLoginDate			, @intSumPrice				, @inyPCRoomLoginDateCount	, @inyHomeLoginDateCount	, @inyLoginDateCount		, @insAveragePlayTime		, @insTotalSessionCount		
, @inyAge					, @insRegisterDays			, @intPastDTSum				, @insTerm1					, @insTerm2					, @insTerm3					, @insTerm4					, @insTerm5					
, @insTerm6					, @insTerm7					, @insTerm8					, @insTerm9					, @insTerm10					
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
5 = A.AccountID -> a.Account? ?? at 2010-12-27
**/

ALTER PROCEDURE [dbo].[P_GetLeavePreventionConnInfo]
	  @intAccountID				int				   --// ??ID
	, @dtmLastLoginDate			datetime	OUTPUT --// ?????
	, @intSumPrice				int			OUTPUT --// ?? ??? ?? ?? ?
	, @inyPCRoomLoginDateCount	tinyint		OUTPUT --// ?? ??? PC? ?? ? ?
	, @inyHomeLoginDateCount	tinyint		OUTPUT --//  ?? ??? ? ?? ? ?
	, @inyLoginDateCount		tinyint		OUTPUT --// ?? ??? ?? ? ?
	, @insAveragePlayTime		smallint	OUTPUT --// ?? ??? ?? Play Time (?)
	, @insTotalSessionCount		smallint	OUTPUT --// ?? ??? ?? Session Count
	, @inyAge					tinyint		OUTPUT --// ??
	, @insRegisterDays			smallint	OUTPUT --// ???????
	, @intPastDTSum				int			OUTPUT --// ?? ? DT
	, @insTerm1					smallint	OUTPUT --// ???1
	, @insTerm2					smallint	OUTPUT --// ???2
	, @insTerm3					smallint	OUTPUT --// ???3
	, @insTerm4					smallint	OUTPUT --// ???4
	, @insTerm5					smallint	OUTPUT --// ???5
	, @insTerm6					smallint	OUTPUT --// ???6
	, @insTerm7					smallint	OUTPUT --// ???7
	, @insTerm8					smallint	OUTPUT --// ???8
	, @insTerm9					smallint	OUTPUT --// ???9
	, @insTerm10				smallint	OUTPUT --// ???10
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue INT
	  , @dtmLastDate	DATETIME

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	SET @dtmLastDate = CONVERT(DATETIME, CONVERT(CHAR(8), DATEADD(DAY, -7, GETDATE()), 112));
	
	SELECT @dtmLastLoginDate		= a.LastLoginDate								-- ?????
		 , @intSumPrice				= SUM(e.ListPrice)								-- ?? ??? ?? ?? ?
		 , @inyPCRoomLoginDateCount = b.PCRoomLoginDateCount						-- ?? ??? PC? ?? ? ?
		 , @inyHomeLoginDateCount	= b.HomeLoginDateCount							-- ?? ??? ? ?? ? ?
		 , @inyLoginDateCount		= b.LoginDateCount								-- ?? ??? ?? ? ?
		 , @insAveragePlayTime		= b.AveragePlayTime								-- ?? ??? ?? Play Time (?)
		 , @insTotalSessionCount	= b.TotalSessionCount							-- ?? ??? ?? Session Count
		 , @inyAge					= DATEDIFF(YYYY, a.BirthDate, GETDATE()) + 1	-- ??
		 , @insRegisterDays			= DATEDIFF(D, a.RegisterDate, GETDATE())		-- ???????
		 , @intPastDTSum			= ISNULL(a.TotalDT, 0)							-- ?? ? DT
	  FROM dbo.Accounts a WITH (NOLOCK)
		   LEFT OUTER JOIN dbo.LeavePreventionConnInfo b WITH (NOLOCK) ON (a.AccountID = b.AccountID)
		   LEFT OUTER JOIN dbo.Characters c WITH (NOLOCK) ON (b.AccountID = c.AccountID)
		   LEFT OUTER JOIN dbo.PurchaseOrders d WITH (NOLOCK) ON (c.CharacterID = d.ReceiverCharacterID AND d.OrderDate >= @dtmLastDate AND d.PayMethodCode = 2 AND d.OrderStatusCode = 2)
		   LEFT OUTER JOIN dbo.PurchaseOrderDetails e WITH (NOLOCK) ON (d.PurchaseOrderID = e.PurchaseOrderID AND e.RefundableFlag = CONVERT(BIT, 0))
	 WHERE a.AccountID = @intAccountID
	 GROUP BY a.LastLoginDate
			, b.PCRoomLoginDateCount
			, b.HomeLoginDateCount
			, b.LoginDateCount
			, b.AveragePlayTime
			, b.TotalSessionCount
			, DATEDIFF(YYYY, a.BirthDate, GETDATE()) + 1
			, DATEDIFF(D, a.RegisterDate, GETDATE())
			, ISNULL(a.TotalDT, 0)
			
	/**_# ??? ???? 1?? ?? SELECT ???.*/
	SELECT @insTerm1 = Term1, @insTerm2 = Term2, @insTerm3 = Term3, @insTerm4 = Term4, @insTerm5 = Term5, @insTerm6 = Term6, @insTerm7 = Term7, @insTerm8 = Term8, @insTerm9 = Term9, @insTerm10 = Term10
	FROM dbo.LeavePreventionLoginTerm WITH (NOLOCK)
	WHERE AccountID = @intAccountID;

END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO
