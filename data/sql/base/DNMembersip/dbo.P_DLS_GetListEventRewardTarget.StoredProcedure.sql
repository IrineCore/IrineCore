USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_GetListEventRewardTarget]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DLS_GetListEventRewardTarget]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DLS_GetListEventRewardTarget] AS' 
END
GO
/**
version : 1
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2012-08-24

description :
	??? ?? ?? ?? ??
result set : 
	EventRewardID / int / ??? ?? ID (FK)
	EventRewardTargetID / int / ??? ?? ?? ID
	AccountName / nvarchar(50) / ?? ?? ??
	AccountID / int / ?? ?? ID
	CharacterName / nvarchar(30) / ?? ??? ??
	CharacterID / bigint / ?? ??? ID
	DeleteFlag / bit / ?? ?? (0=??, 1=??)
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_DLS_GetListEventRewardTarget]
	  @intEventRewardID int --// ??? ?? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	/**_# ??? ?? ?? ?? ?? */
	SELECT E.EventRewardID, E.EventRewardTargetID, A.AccountName, E.AccountID, C.CharacterName, E.CharacterID, E.DeleteFlag
	  FROM dbo.EventRewardTargets E WITH (NOLOCK)
		   INNER JOIN dbo.Accounts A WITH (NOLOCK) ON (E.AccountID = A.AccountID)
		   LEFT OUTER JOIN dbo.Characters C WITH (NOLOCK) ON (E.CharacterID = C.CharacterID)
	 WHERE E.EventRewardID = @intEventRewardID;

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
