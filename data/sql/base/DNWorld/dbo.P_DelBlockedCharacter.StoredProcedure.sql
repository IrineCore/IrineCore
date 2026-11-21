USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelBlockedCharacter]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-18
description : ???? ?? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103140 = ?? ?? ??? ???? ?? ?? ??? ???? ?? ???? ????
history :
2 = TF_Split? IF_Split?? ?? at 2010-12-21
3 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-01
**/

CREATE PROCEDURE [dbo].[P_DelBlockedCharacter]
	  @inbCharacterID bigint --// ??? ID
	, @vchBlockedCharacterIDs varchar(8000) --// ?? ??? ??? ID ???

--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @i int;
DECLARE @tblBlockedCharacters table (BlockedCharacterID bigint NOT NULL PRIMARY KEY);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ????? ??? ID? ??????.*/
	INSERT @tblBlockedCharacters (BlockedCharacterID)
	SELECT CAST(string AS bigint)
	FROM dbo.TF_Split(@vchBlockedCharacterIDs, ',')
	WHERE string > ''
	OPTION (KEEPFIXED PLAN);

	SET @i = @@ROWCOUNT;

	BEGIN TRANSACTION;

	/**_# [BlockedCharacters]????? DELETE???.*/
	DELETE dbo.BlockedCharacters
	FROM dbo.BlockedCharacters BCH
		INNER JOIN @tblBlockedCharacters T ON BCH.BlockedCharacterID = T.BlockedCharacterID
	WHERE BCH.CharacterID = @inbCharacterID;

	/**_# ?? ?? ??? ???? ?? ?? ??? ???? ?? ???? ??? 103140? ?????.*/
	IF @i <> @@ROWCOUNT
	BEGIN
		SET @intReturnValue = 103140;
		GOTO ErrorHandler;
	END

	COMMIT TRANSACTION;
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
