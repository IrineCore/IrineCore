USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetCharacterCheckInfo]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-02-18
description : ??? ??? ??? ??? ????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
1030000028 = ???? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_DRS_GetCharacterCheckInfo]
	  @inbCharacterID bigint --// ??? ID	
	, @inyCharacterLevel tinyint = NULL OUTPUT --// ?????
	, @intCharacterExp int = NULL OUTPUT --// ??????
	, @inbCoin bigint = NULL OUTPUT --// ??
	, @inbWarehouseCoin bigint = NULL OUTPUT --// ????
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
	/**_# [CharacterStatus] ???? SELECT??.*/
	SELECT @inyCharacterLevel = CharacterLevel, @intCharacterExp = CharacterExp, @inbCoin = Coin, @inbWarehouseCoin = WarehouseCoin
	FROM dbo.CharacterStatus WITH (NOLOCK, INDEX(PK_CharacterStatus), FORCESEEK)
	WHERE CharacterID = @inbCharacterID

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 1030000028;
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
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO
