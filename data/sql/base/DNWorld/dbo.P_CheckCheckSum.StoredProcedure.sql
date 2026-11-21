USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_CheckCheckSum]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2013-04-16
description : ??? ??? ?? ?????.
result set : LastLoginDate/datetime2, LastLogoutDate/datetime2, CheckSumBin/binary(4)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103394 = ??? ?? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_CheckCheckSum]
	  @inbCharacterID bigint --// ??? ID	
	, @binCheckSumBin binary(4) --// ???
 	, @vchCheckSumPlainText varchar(100) = NULL OUTPUT --// ??? ?? ??? (??? ??? ? ?????)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @bitCheckSumFlag bit
	, @binCheckSumBin_DB binary(4);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	BEGIN TRANSACTION;

	UPDATE dbo.CharacterStatus
	SET @bitCheckSumFlag = CheckSumFlag
		, @binCheckSumBin_DB = CheckSumBin
		, CheckSumFlag = 0
	WHERE CharacterID = @inbCharacterID;

	IF @binCheckSumBin_DB <> @binCheckSumBin AND @bitCheckSumFlag = 1
	BEGIN
		SELECT @vchCheckSumPlainText = CheckSumPlainText
		FROM dbo.CheckSumPlainTexts WITH (NOLOCK, INDEX(PK_CheckSumPlainTexts), FORCESEEK)
		WHERE CharacterID = @inbCharacterID
		OPTION (KEEPFIXED PLAN);

		SET @intReturnValue = 103394;
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
