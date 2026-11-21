USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListLockedItems]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 10
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-01-21
description : ??? ??? ??? ?????.
result set : ItemLocationCode/tinyint, ItemLocationIndex/tinyint, UnlockRequestDate/datetime2(0), LockedDate/datetime2(0), UnlockDate/datetime2(0), ItemSerial/bigint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListLockedItems]
	  @inbOwnerCharacterID bigint --// ?? ??? ID		
	, @intVersion int = 3 --// ??
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
	/**_# [LockedItems] ???? SELECT???.*/	
	SELECT ItemLocationCode, ItemLocationIndex, UnlockRequestDate, LockedDate, UnlockDate, ItemSerial
	FROM dbo.LockedItems WITH (NOLOCK, INDEX(PK_LockedItems), FORCESEEK)
	WHERE OwnerCharacterID = @inbOwnerCharacterID
	OPTION (MAXDOP 1);

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
