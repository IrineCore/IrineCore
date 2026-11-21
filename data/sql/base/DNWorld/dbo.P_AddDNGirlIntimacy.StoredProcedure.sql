USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddDNGirlIntimacy]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 12
author : dailykim
e-mail : dailykim@eyedentitygames.com
created date : 2013-08-01
description : ??? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103414 = ?? ??? ??????.
103102 = ???? ???? ????.
103419 = ??? ???? ?? ????? ???? ??? ? ????.
**/

CREATE PROCEDURE [dbo].[P_AddDNGirlIntimacy]
	  @inbCharacterID bigint --// ???ID
	, @nvcDNGirlCharacterName nvarchar(30) --// ??? ?????
	, @intAddedIntimacy int --// ??? ???
	, @intIntimacyToFan int --// ?? ?? ?? ???
	, @bitFanFlag bit OUTPUT --// 0=? ??, 1= ?	
	, @intCurrentIntimacy int OUTPUT --// ??? ???
	, @intMaxIntimacy int --// ?? ??? ??
	
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @intIntimacy int
	  , @intFanIntimacy int
	  , @dt2Now datetime2(0) = GETDATE()
	  , @inbDNGirlCharacterID bigint
	  , @inyAccountLevelCode tinyint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# ???? ???ID? ????.*/
	SELECT @inbDNGirlCharacterID = CharacterID, @inyAccountLevelCode= AccountLevelCode FROM dbo.Characters WITH (NOLOCK) WHERE CharacterName = @nvcDNGirlCharacterName AND DeleteFlag = 0;
	
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103102;
		GOTO ErrorHandler;
	END
	
	/**_# ??? ???? ??? ?????.*/
	IF @inyAccountLevelCode <> 120
	BEGIN
		SET @intReturnValue = 103419;
		GOTO ErrorHandler;
	END
	
	
	/**_# ????? ?????? ????? ?????.*/
	IF EXISTS (SELECT * FROM dbo.BlockedDNGirls WITH (NOLOCK) WHERE DNGirlCharacterID = @inbDNGirlCharacterID)
	BEGIN
		SET @intReturnValue = 103414;
		GOTO ErrorHandler;
	END	
	
	/**_# [DNGirlIntimacy] ???? UPSERT???.*/
	SELECT @intIntimacy = ISNULL(Intimacy, 0)
	FROM dbo.DNGirlIntimacy WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID AND DNGirlCharacterID = @inbDNGirlCharacterID;
	
	IF @@ROWCOUNT = 0
	BEGIN
		IF @intAddedIntimacy >= @intMaxIntimacy
			SET @intAddedIntimacy = @intMaxIntimacy;
			
		IF @intAddedIntimacy >= @intIntimacyToFan
			SET @bitFanFlag = CAST (1 AS bit);
		ELSE
			SET @bitFanFlag = CAST (0 AS bit);	
			
		INSERT dbo.DNGirlIntimacy (CharacterID, DNGirlCharacterID, Intimacy, FanFlag, UpdateDate)
		SELECT @inbCharacterID, @inbDNGirlCharacterID, @intAddedIntimacy, @bitFanFlag, @dt2Now;
		
		SET @intCurrentIntimacy = @intAddedIntimacy;		
		SET @bitFanFlag = CAST (0 AS bit);
	END
	ELSE
	BEGIN		
		SET @intCurrentIntimacy = @intIntimacy + @intAddedIntimacy;
		
		IF @intCurrentIntimacy >= @intMaxIntimacy
			SET @intCurrentIntimacy = @intMaxIntimacy;
		
		IF @intCurrentIntimacy >= @intIntimacyToFan
			SET @bitFanFlag = CAST (1 AS bit);
		ELSE
			SET @bitFanFlag = CAST (0 AS bit);	
		
		UPDATE dbo.DNGirlIntimacy
		SET Intimacy = @intCurrentIntimacy, FanFlag = @bitFanFlag, UpdateDate = @dt2Now
		WHERE CharacterID = @inbCharacterID AND DNGirlCharacterID = @inbDNGirlCharacterID;	
		
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
