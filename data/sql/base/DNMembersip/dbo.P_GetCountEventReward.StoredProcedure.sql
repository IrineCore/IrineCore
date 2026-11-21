USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetCountEventReward]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetCountEventReward]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetCountEventReward] AS' 
END
GO
/**
version : 4
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2012-08-23

description :
	??? ?? ?? ??
result set :
	EventRewardCount / int / ??? ?? ??
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_GetCountEventReward]
	  @intAccountID int --// ?? ID
	, @inbCharacterID bigint --// ??? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @sdtNow smalldatetime = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	/**_# ??? ?? ?? ?? */
	SELECT COUNT(*) AS EventRewardCount
	  FROM dbo.EventRewards E WITH (NOLOCK)
	 WHERE (E.ReserveSendDate <= @sdtNow AND @sdtNow <= E.ExpirationDate)
	   AND (E.DeleteFlag = CONVERT(bit, 0))
	   AND (   (E.TargetTypeCode = 1 AND NOT EXISTS (SELECT * FROM dbo.EventRewardReceivers R WITH (NOLOCK) WHERE R.EventRewardID = E.EventRewardID AND R.AccountID = @intAccountID))
			OR (E.TargetTypeCode = 2 AND EXISTS (SELECT * FROM dbo.EventRewardTargets T WITH (NOLOCK) WHERE T.EventRewardID = E.EventRewardID AND T.AccountID = @intAccountID)
									 AND NOT EXISTS (SELECT * FROM dbo.EventRewardReceivers R WITH (NOLOCK) WHERE R.EventRewardID = E.EventRewardID AND R.AccountID = @intAccountID))
			OR (E.TargetTypeCode = 3 AND NOT EXISTS (SELECT * FROM dbo.EventRewardReceivers R WITH (NOLOCK) WHERE R.EventRewardID = E.EventRewardID AND R.CharacterID = @inbCharacterID))
			OR (E.TargetTypeCode = 4 AND EXISTS (SELECT * FROM dbo.EventRewardTargets T WITH (NOLOCK) WHERE T.EventRewardID = E.EventRewardID AND T.CharacterID = @inbCharacterID)
									 AND NOT EXISTS (SELECT * FROM dbo.EventRewardReceivers R WITH (NOLOCK) WHERE R.EventRewardID = E.EventRewardID AND R.CharacterID = @inbCharacterID))
			OR (E.TargetTypeCode = 5 AND NOT EXISTS (SELECT * FROM dbo.EventRewardReceivers R WITH (NOLOCK) WHERE R.EventRewardID = E.EventRewardID AND R.CharacterID = @inbCharacterID))
		   )
	OPTION (KEEPFIXED PLAN);

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
