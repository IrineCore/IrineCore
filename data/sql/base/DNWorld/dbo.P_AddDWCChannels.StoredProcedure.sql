USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddDWCChannels]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 14
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-01-15
description : DWC ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103362 = DWC ?? ??? ? ?? ??? ? ????.
103363 = ?? ??? ??? ???? ?? ? ??? ??? ? ????.
103377 = ?? ?? DWC ???? ??? ?? ?? ?? ??? ? ????.
**/

CREATE PROCEDURE [dbo].[P_AddDWCChannels]
	  @dt2DWCStartTime datetime2(0) --// DWC ?? ??
	, @dt2DWCEndTime datetime2(0) --// DWC ?? ??
	, @bitEnableFlag bit --// 0=????, 1??
	, @intLadderTime1 int --// ??????1 ex) 07:30~10:00 ????? 7301000 ??!
	, @intLadderTime2 int = NULL --// (optional)??????2 ex) 19:00~21:00 ????? 19002100 ??!
	, @inyLimitCharacterLevel tinyint = NULL --// DWC ??? ?? ?? ??(????? DWC ON/OFF??? ????? ??? ????!!!)
	, @intDWCChannelID int = NULL OUTPUT --// DWC ????
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @dt2Now datetime2(0)
	  , @insDWCSequence smallint

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF EXISTS(SELECT * FROM dbo.DWCChannels WHERE DWCEndTime >= @dt2Now AND EnableFlag = 1)
	BEGIN
		SET @intReturnValue = 103363;
		GOTO ErrorHandler;
	END

	IF EXISTS (SELECT * FROM dbo.DWCChannels WHERE EnableFlag = 0 AND DataInitFlag = 0)
	BEGIN
		SET @intReturnValue = 103377;
		GOTO ErrorHandler;
	END

	SELECT @insDWCSequence = MAX(DWCSequence) + 1
	FROM dbo.DWCChannels WITH(NOLOCK)

	/**_# [DWCChannels] ???? INSERT???.*/
	INSERT dbo.DWCChannels (DWCSequence, DWCStartTime, DWCEndTime, EnableFlag, LadderTime1, LadderTime2, LimitCharacterLevel)
	VALUES (ISNULL(@insDWCSequence, 1), @dt2DWCStartTime, @dt2DWCEndTime, @bitEnableFlag, @intLadderTime1, @intLadderTime2, @inyLimitCharacterLevel);

	SET @intDWCChannelID = ISNULL(@insDWCSequence, 1);
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2601 AND ERROR_MESSAGE() LIKE N'%IX_UNF_DWCChannels_EnableFlag%'
		SET @intReturnValue = 103362;

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
