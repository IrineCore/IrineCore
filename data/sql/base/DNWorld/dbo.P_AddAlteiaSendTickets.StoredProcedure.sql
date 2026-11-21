USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddAlteiaSendTickets]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-10-18
description : ???? ???? ? ??? ????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103369 = ?? ????? ?? ???? ??? ?????.
103370 = ??? ???? ???? ????.
103371 = ?? ???? ???? ????.
103372 = ????? ??? ?? ????? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddAlteiaSendTickets]
	  @inbCharacterID bigint --// ???ID
	, @inbReceiveCharacterID bigint --// ?? ???ID
	, @inyTicketLimitCount tinyint --// ????? ??? ?? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @dt2SendDate datetime2(0) = GETDATE()--// ??? ??
	  , @inbSendTicketCount bigint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF NOT EXISTS(SELECT * FROM dbo.Characters WITH(NOLOCK) WHERE CharacterID = @inbCharacterID)
	BEGIN
		SET @intReturnValue = 103350;
		GOTO ErrorHandler;
	END
	
	IF NOT EXISTS(SELECT * FROM dbo.Characters WITH(NOLOCK) WHERE CharacterID = @inbReceiveCharacterID)
	BEGIN
		SET @intReturnValue = 103351;
		GOTO ErrorHandler;
	END

	SELECT @inbSendTicketCount = SendTicketCount 
	FROM dbo.VIX_AlteiaSendTicketTotals WITH (NOEXPAND, NOLOCK) 
	WHERE CharacterID = @inbCharacterID;

	IF (@inbSendTicketCount >= @inyTicketLimitCount)
	BEGIN
		SET @intReturnValue = 103352;
		GOTO ErrorHandler;
	END

	/**_# [AlteiaSendTickets] ???? INSERT???.*/
	INSERT dbo.AlteiaSendTickets (CharacterID, ReceiveCharacterID, SendDate)
	VALUES (@inbCharacterID, @inbReceiveCharacterID, @dt2SendDate);
END TRY
BEGIN CATCH

	IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_AlteiaSendTickets%'
		SET @intReturnValue = 103369;
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
