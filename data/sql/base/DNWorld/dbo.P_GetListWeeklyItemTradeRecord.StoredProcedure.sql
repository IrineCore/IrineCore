USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListWeeklyItemTradeRecord]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-07-17
description : ?? ?? ??? ?? ??? ?????.
result set : 
		version 1 : ItemID/int, ItemCount/int
		version 2 : ItemID/int, ItemCount/int, ShopID/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListWeeklyItemTradeRecord]
	  @inbCharacterID bigint --// ???ID
	, @intVersion int = 1 --// ??? ??	
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
	/**_# [WeeklyItemTradeRecord] ???? SELECT???.*/

	IF @intVersion = 2	
		SELECT ItemID, ItemCount, ShopID
		  FROM dbo.WeeklyItemTradeRecord WITH (NOLOCK, INDEX(IX_UC_WeeklyItemTradeRecord_CharacterID_ItemID_ShopID), FORCESEEK)
		 WHERE CharacterID = @inbCharacterID
	    OPTION (MAXDOP 1);
	ELSE
		SELECT ItemID, ItemCount
		  FROM dbo.WeeklyItemTradeRecord WITH (NOLOCK, INDEX(IX_UC_WeeklyItemTradeRecord_CharacterID_ItemID_ShopID), FORCESEEK)
		 WHERE CharacterID = @inbCharacterID
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
