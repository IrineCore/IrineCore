USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_UpsNestClear]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-27
description : ??? ??? ??? UPSERT???.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
history :
2 = (43652) PC? ?? ??? ?? ?? ?? by ??? at 2011-12-13
3 = ??? ?? ??? ?? ?? ??. by ??? at 2012-12-05
4 = 3? ?? ?? by ??? at 2012-12-05
**/

CREATE PROCEDURE [dbo].[P_UpsNestClear]
	  @inbCharacterID bigint --// ??? ID
	, @intMapID int --// ? ID
	, @inyLocationType tinyint = 1 --// 1=??, 2=PC?
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
	UPDATE dbo.NestClear
	SET ClearCount += CASE @inyLocationType WHEN 1 THEN 1 ELSE 0 END
		, PCRoomClearCount += CASE @inyLocationType WHEN 2 THEN 1 ELSE 0 END
		, LastClearDate = CAST(GETDATE() AS date)
	WHERE CharacterID = @inbCharacterID AND MapID = @intMapID;

	IF @@ROWCOUNT = 0
		INSERT dbo.NestClear (CharacterID, MapID, ClearCount, PCRoomClearCount, LastClearDate)
		VALUES (@inbCharacterID, @intMapID, CASE @inyLocationType WHEN 1 THEN 1 ELSE 0 END, CASE @inyLocationType WHEN 2 THEN 1 ELSE 0 END, CAST(GETDATE() AS date));
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%FK_Characters_NestClear_CharacterID%'
		SET @intReturnValue = 103102;

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
