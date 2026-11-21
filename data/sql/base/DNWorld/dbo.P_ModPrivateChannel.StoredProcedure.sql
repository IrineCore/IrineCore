USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModPrivateChannel]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-07-11
description : ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103329 = ?? ????? ???? ????.
103327 = ?? ???? ???? ???.
**/

CREATE PROCEDURE [dbo].[P_ModPrivateChannel]
	  @inbPrivateChannelID bigint --// ????ID
	, @nvcPrivateChannelName nvarchar(10) = NULL --// ??????
	, @inbMasterCharacterID bigint = NULL  --// ??ID
	, @intChannelPassword int = NULL  --// ??????	
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
	/**_# [PrivateChannels] ???? UPDATE???.*/
	UPDATE dbo.PrivateChannels
	   SET PrivateChannelName = COALESCE(@nvcPrivateChannelName, PrivateChannelName),
		   MasterCharacterID = COALESCE(@inbMasterCharacterID, MasterCharacterID),	       
	       ChannelPassword = COALESCE(@intChannelPassword, ChannelPassword)
	 WHERE PrivateChannelID = @inbPrivateChannelID;

	IF @@ROWCOUNT = 0 
	BEGIN
		SET @intReturnValue = 103329;
		GOTO ErrorHandler;		
	END
	
END TRY
BEGIN CATCH

	IF ERROR_NUMBER() = 2601 AND ERROR_MESSAGE() LIKE N'%IX_UN_%'
		SET @intReturnValue = 103327;

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
