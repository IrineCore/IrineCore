USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_AddDNGirlRewardResult]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : dailykim
e-mail : dailykim@eyedentitygames.com
created date : 2013-08-08
description : ??? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DLS_AddDNGirlRewardResult]
	  @inbDNGirlCharacterID bigint --// ??? ??? ID
	, @inyRewardTypeCode tinyint = NULL --// ??? ?? ????
	, @bitDNGirlRewardFlag bit = NULL --// 0=?????, 1=??????
	, @bitFanRewardFlag bit = NULL --// 0=?????, 1=??????
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
	/**_# [DNGirlRewardResults] ???? UPDATE???.*/
	UPDATE dbo.DNGirlRewardResults
	SET RewardTypeCode = COALESCE(@inyRewardTypeCode, RewardTypeCode)
	  , DNGirlRewardFlag = COALESCE(@bitDNGirlRewardFlag, DNGirlRewardFlag)
	  , FanRewardFlag = COALESCE(@bitFanRewardFlag, FanRewardFlag)
	WHERE DNGirlCharacterID = @inbDNGirlCharacterID;
	
	/**_# UPDATE ? Row? ?? ?? INSERT???.*/
	IF @@ROWCOUNT = 0
		INSERT dbo.DNGirlRewardResults (DNGirlCharacterID, RewardTypeCode, DNGirlRewardFlag, FanRewardFlag)
		VALUES (@inbDNGirlCharacterID, @inyRewardTypeCode, @bitDNGirlRewardFlag, @bitFanRewardFlag);
		
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
