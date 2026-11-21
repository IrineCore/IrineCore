USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListNestClearCount]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 9
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-23
description : ??? ?? ?? ??? ??? ?????.
result set : 
	@intVersion = 3 : MapID/int, ClearCount/tinyint
	@intVersion = 5 : MapID/int, ClearCount/tinyint, PCRoomClearCount/tinyint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103109 = ???? ???? ????.
history :
2 = ??? ??? ???? ??? ???? ?? at 2010-07-27
3 = ??? ??? ??  by 20110412.ohtack
4 = (43652) PC? ?? ??? ?? ?? ?? by ??? at 2011-12-13
5 = (43652) PC? ?? ??? ?? ?? ??. input param. @intVersion ?? by ??? at 2011-12-13
6 = ??? ?? ??? ?? ?? ??. by ??? at 2012-12-05
8~9 = 6??? ?? by ??? at 2012-12-05
**/

CREATE PROCEDURE [dbo].[P_GetListNestClearCount]
	  @inbCharacterID bigint --// ??? ID
	, @intVersion int = 3 --// ??? SP? ??
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
	/**_# [NestClear] ???? SELECT???.*/
	IF @intVersion = 3
		SELECT MapID, ClearCount
		FROM dbo.NestClear WITH (NOLOCK, INDEX(PK_NestClear), FORCESEEK)
		WHERE CharacterID = @inbCharacterID
		ORDER BY CharacterID, MapID
		OPTION (KEEPFIXED PLAN);

	ELSE IF @intVersion = 5
		SELECT MapID, ClearCount, PCRoomClearCount
		FROM dbo.NestClear WITH (NOLOCK, INDEX(PK_NestClear), FORCESEEK)
		WHERE CharacterID = @inbCharacterID
		OPTION (KEEPFIXED PLAN);

	/**_# ???? ???? ??? 103109? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103109;
		GOTO ErrorHandler;
	END
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
