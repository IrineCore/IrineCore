USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModDWCChannel]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 15
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-01-15
description : DWC ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103362 = DWC ?? ??? ? ?? ??? ? ????.
103379 = ?? ??? DWC ??? ????? ????.
**/

CREATE PROCEDURE [dbo].[P_ModDWCChannel]
	  @intDWCChannelID int --// DWC ??ID
	, @dt2DWCStartTime datetime2(0) --// DWC ?? ??
	, @dt2DWCEndTime datetime2(0) --// DWC ?? ??
	, @inyStatusCode tinyint --// 1=??, 2=??, 3=????, 4=??(??!! ????? ??????? ?????.)
	, @intLadderTime1 int --//??????1 ex) 07:30~10:00 ????? 7301000 ??!
	, @intLadderTime2 int = NULL --//??????1 ex) 07:30~10:00 ????? 7301000 ??!
	, @inyLimitCharacterLevel tinyint = NULL --//??? ?? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @dt2Now datetime2(0) = GETDATE()
	  , @inyCurrentStatusCode tinyint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT @inyCurrentStatusCode = StatusCode
	FROM dbo.DWCChannels WITH(NOLOCK) 
	WHERE DWCSequence = @intDWCChannelID

	/**_# ????? ??????? ????? ?? ??? ??? ? ????.*/
	/**_# ???? ????? ??? ??? ? ????.*/
	IF (@inyStatusCode IN (1, 3, 4) AND @inyCurrentStatusCode <> 2) OR @inyCurrentStatusCode = 3 
	BEGIN
		IF @inyStatusCode <> @inyCurrentStatusCode 
		BEGIN
			SET @intReturnValue = 103379;
			GOTO ErrorHandler;
		END
	END

	/**_# [DWCChannels] ???? UPDATE???.*/
	UPDATE dbo.DWCChannels 
	SET DWCStartTime = @dt2DWCStartTime
      , DWCEndTime = @dt2DWCEndTime
	  , EnableFlag = CASE WHEN @inyStatusCode = 3 THEN 0 WHEN @inyStatusCode = 4 THEN 1 ELSE EnableFlag END
	  , StatusCode = CASE WHEN @inyStatusCode = 4 THEN 1 ELSE @inyStatusCode END
	  , LadderTime1 = @intLadderTime1
	  , LadderTime2 = @intLadderTime2
	  , DWCStopTime = CASE WHEN @inyStatusCode = 2 THEN @dt2Now WHEN @inyStatusCode = 4 THEN DWCStopTime ELSE NULL END
	  , DWCReopenTime = CASE WHEN @inyStatusCode = 4 THEN @dt2Now ELSE NULL END
	  , LimitCharacterLevel = @inyLimitCharacterLevel
	WHERE DWCSequence = @intDWCChannelID

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
