USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DelCharacter]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DelCharacter]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DelCharacter] AS' 
END
GO
/**
version : 8
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-06
description : ???? ?????. (DeleteFlag? UPDATE???.)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101117 = ???? ???? ????.
101118 = ?? ??? ??????.
history :
7 = ??? ??? NULL? ??? ?? ?? ?? at 2010-12-02
8 = input param. @bitDeleteByAdminFlag ?? at 2010-12-02
**/

ALTER PROCEDURE [dbo].[P_DelCharacter]
	  @inbCharacterID bigint --// ??? ID
	, @vchDeletePrivateIP varchar(15) --// ???? ??? ?? IP ??
	, @vchDeletePublicIP varchar(15) --// ???? ??? ?? IP ??
	, @bitCharacterNameReserveFlag bit --// ??? ?? ?? ??. 0=???? ??, 1=???
	, @dt2DeleteDate datetime2(0) --// ???? ??? ??
	, @bitDeleteByAdminFlag bit = 0 --// 0=?? ?? ??, 1=??? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @bitDeleteFlag bit
	, @nvcCharacterName nvarchar(30);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	BEGIN TRANSACTION;

	/**_# [Characters] ????? DELETE???.*/
	UPDATE dbo.Characters
	SET @bitDeleteFlag = DeleteFlag
		, @nvcCharacterName = CharacterName
		, DeleteFlag = 1
		, CharacterName = CASE @bitCharacterNameReserveFlag WHEN 1 THEN CharacterName ELSE NULL END
		, DeleteByAdminFlag = @bitDeleteByAdminFlag
	WHERE CharacterID = @inbCharacterID;

	/**_# ???? ???? ??? 101117? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 101117;
		GOTO ErrorHandler;
	END

	/**_# ?? ??? ????? 101118? ?????.*/
	IF @bitDeleteFlag = 1
	BEGIN
		SET @intReturnValue = 101118;
		GOTO ErrorHandler;
	END

	/**_# [DeletedCharacters] ???? INSERT???.*/
	INSERT dbo.DeletedCharacters (CharacterID, CharacterName, DeleteDate, DeletePrivateIP, DeletePublicIP)
	VALUES (@inbCharacterID, ISNULL(@nvcCharacterName, N''), @dt2DeleteDate, dbo.FN_IP2IPNumber(@vchDeletePrivateIP), dbo.FN_IP2IPNumber(@vchDeletePublicIP));

	COMMIT TRANSACTION;

	INSERT dbo.CharacterDeleteReviveLogs (LogDate, CharacterID, DeleteFlag, PrivateIP, PublicIP)
	VALUES (@dt2DeleteDate, @inbCharacterID, 1, dbo.FN_IP2IPNumber(@vchDeletePrivateIP), dbo.FN_IP2IPNumber(@vchDeletePublicIP));
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_DeletedCharacters%'
		SET @intReturnValue = 101118;

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
