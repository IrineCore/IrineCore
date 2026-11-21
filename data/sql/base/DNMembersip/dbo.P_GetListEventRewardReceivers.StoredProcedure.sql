USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListEventRewardReceivers]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListEventRewardReceivers]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListEventRewardReceivers] AS' 
END
GO
/**
version : 1
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-10-18
description : ????? ??? ???? ?????(DOLIS)
result set : AccountName nvarchar(50), CharacterName nvarchar(30), CharacterID bigint, ReceiveDate datetime2(0)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

ALTER PROCEDURE [dbo].[P_GetListEventRewardReceivers]
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
	
	SELECT C.AccountName, B.CharacterName, B.CharacterID, A.ReceiveDate
	FROM dbo.EventRewardReceivers A WITH (NOLOCK) 
		INNER JOIN dbo.Characters B WITH(NOLOCK) ON A.CharacterID = B.CharacterID
		INNER JOIN dbo.Accounts C WITH(NOLOCK) ON A.AccountID = C.AccountID
	WHERE EventRewardID = @intEventRewardID
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
