USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetDNGirlIntimacy]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : dailykim
e-mail : dailykim@eyedentitygames.com
created date : 2013-08-01
description : ???? ??? ???? ?????. (?? ??? ??)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103410 = ?? ???? ?? ????.
**/

CREATE PROCEDURE [dbo].[P_GetDNGirlIntimacy]
	  @inbCharacterID bigint --// ???ID
	, @inbDNGirlCharacterID bigint OUTPUT --// ??? ???ID
	, @intIntimacy int OUTPUT --// ???	
	, @nvcDNGirlCharacterName nvarchar(30) OUTPUT --// ??? ?????
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
	/**_# [DNGirlIntimacy] ???? SELECT???.*/
	SELECT @inbDNGirlCharacterID = DG.DNGirlCharacterID, @intIntimacy = DG.Intimacy, @nvcDNGirlCharacterName = CH.CharacterName
	FROM dbo.DNGirlIntimacy DG WITH (NOLOCK) INNER JOIN dbo.Characters CH WITH (NOLOCK) ON DG.DNGirlCharacterID = CH.CharacterID
	WHERE DG.CharacterID = @inbCharacterID AND DG.FanFlag = CAST (1 AS bit);
	
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103410;
		GOTO ErrorHandler;
	END
END TRY
BEGIN CATCH	

	IF XACT_STATE() <> 0
		ROLLBACK TRANSACTION;

	IF @intReturnValue IS NULL OR @intReturnValue = 0
		EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

	ELSE
		GOTO ErrorHandler;

	RETURN @intReturnValue;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

RETURN @intReturnValue;
GO
