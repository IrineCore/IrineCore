USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListAlteiaSendTickets]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-10-18
description : ???? ??? ?? ??? ?????.
result set : ReceiveCharacterID/bigint, SendDate/datetime2(0)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListAlteiaSendTickets]
	  @inbCharacterID bigint --// ???ID
	, @intSendTicketCount int OUTPUT --//?? ???? ?
	, @intReceiveTicketCount int OUTPUT--//?? ???? ?
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @dt2SendDate datetime2(0); --// ??? ??

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [AlteiaSendTickets] ???? ?????.*/
	SELECT ReceiveCharacterID, SendDate 
	FROM dbo.AlteiaSendTickets WITH(NOLOCK, INDEX(PK_AlteiaSendTickets), FORCESEEK)
	WHERE CharacterID = @inbCharacterID
	OPTION (KEEPFIXED PLAN);

	SELECT @intReceiveTicketCount = ReceiveTicketCount 
	FROM dbo.VIX_AlteiaReceiveTicketTotals WITH(NOEXPAND, NOLOCK)
	WHERE CharacterID = @inbCharacterID;
	
	SELECT @intSendTicketCount = SendTicketCount 
	FROM dbo.VIX_AlteiaSendTicketTotals WITH(NOEXPAND, NOLOCK)
	WHERE CharacterID = @inbCharacterID;

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
