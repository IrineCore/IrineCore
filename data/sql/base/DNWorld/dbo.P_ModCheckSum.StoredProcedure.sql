USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModCheckSum]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2013-04-16
description : ??? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
1030000028 = ???? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_ModCheckSum]
	  @inbCharacterID bigint --// ??? ID	
	, @binCheckSumBin binary(4) --// ???
	, @vchCheckSumPlainText varchar(100) = NULL --// ??? ?? ???
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
	/**_# [CharacterStatus] ???? UPDATE???.*/
	UPDATE dbo.CharacterStatus 
	SET CheckSumBin = @binCheckSumBin
		, CheckSumFlag = 1
	WHERE CharacterID = @inbCharacterID;
	
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 1030000028;
		GOTO ErrorHandler;	
	END

	/**_# [CheckSumPlainTexts] ???? UPSERT ???. */
	IF @vchCheckSumPlainText IS NOT NULL
	BEGIN
		UPDATE dbo.CheckSumPlainTexts
		SET CheckSumPlainText = @vchCheckSumPlainText
			, LastModifyDate = GETDATE()
		FROM dbo.CheckSumPlainTexts WITH (INDEX(PK_CheckSumPlainTexts), FORCESEEK)
		WHERE CharacterID = @inbCharacterID
		OPTION (KEEPFIXED PLAN);

		IF @@ROWCOUNT = 0
			INSERT dbo.CheckSumPlainTexts (CharacterID, CheckSumPlainText, LastModifyDate)
			VALUES (@inbCharacterID, @vchCheckSumPlainText, GETDATE());
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
