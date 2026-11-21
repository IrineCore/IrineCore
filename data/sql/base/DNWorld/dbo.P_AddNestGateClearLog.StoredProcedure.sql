USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddNestGateClearLog]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-06-19
description : ??? ?? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddNestGateClearLog]
	  @inbRoomID bigint --// ??? ? ID
    , @bitClearFlag bit --// 0=??, 1=???
	, @inbCharacterID bigint = NULL --// ??? ID
	, @inyJobCode tinyint = NULL --// //1=???, 2=??, 3=????, 4=???, 5=????, 10=???, 11=?????, 12=????, 13=???, 14=?????, 15=????, 16=??, 17=??????, 18=????, 19=??, 20=???, 21=??, 22=????, 23=??????, 24=???, 25=????, 26=??????, 27=?????, 28=?????, 29=????, 30=????, 31=????, 32=????, 33=???, 34=?????, 35=????, 36=?????, 37=???, 38=????, 39=?????, 40=??????, 41=???, 42=?????, 43=???, 44=????, 45=?????, 46=????, 47=????, 48=?????, 49=?????, 50=???, 51=???
	, @inyCharacterLevel tinyint = NULL --// ??? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
      , @inbNestGateStartID bigint
	  , @intLogTableSuffix int
	  , @dt2NestGateStartDate datetime2(0)

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT TOP (1) @inbNestGateStartID = NestGateStartID, @dt2NestGateStartDate = NestGateStartDate
	FROM dbo.NestGateStartIDs WITH (NOLOCK, INDEX(IX_NN_NestGateStartIDs_RoomID), FORCESEEK)
	WHERE RoomID = @inbRoomID
	ORDER BY NestGateStartID DESC
	OPTION(KEEPFIXED PLAN);
	
	IF @inbNestGateStartID IS NULL
		RETURN 0;

	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2NestGateStartDate);

	IF @intLogTableSuffix = 0
		INSERT dbo.NestGateClearLogs0 (NestGateStartID, CharacterID, JobCode, CharacterLevel, ClearFlag)
		VALUES (@inbNestGateStartID, ISNULL(@inbCharacterID, 0), ISNULL(@inyJobCode, 0), ISNULL(@inyCharacterLevel, 0), @bitClearFlag);
	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.NestGateClearLogs1 (NestGateStartID, CharacterID, JobCode, CharacterLevel, ClearFlag)
		VALUES (@inbNestGateStartID, ISNULL(@inbCharacterID, 0), ISNULL(@inyJobCode, 0), ISNULL(@inyCharacterLevel, 0), @bitClearFlag);
	ELSE
		INSERT dbo.NestGateClearLogs2 (NestGateStartID, CharacterID, JobCode, CharacterLevel, ClearFlag)
		VALUES (@inbNestGateStartID, ISNULL(@inbCharacterID, 0), ISNULL(@inyJobCode, 0), ISNULL(@inyCharacterLevel, 0), @bitClearFlag);

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
