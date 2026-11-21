USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddPartyEndLog]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-04-05
description : ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddPartyEndLog]
	  @inbPartyID bigint --// ??? ?? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @inbPartyStartID bigint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [PartyStartIDs] ???? UPDATE???.*/
	SELECT TOP (1) @inbPartyStartID = PartyStartID
	FROM dbo.PartyStartIDs WITH (NOLOCK)
	WHERE PartyID = @inbPartyID
	ORDER BY PartyStartID DESC;

	UPDATE dbo.PartyStartIDs
	SET PartyEndDate = GETDATE()
	WHERE PartyStartID = @inbPartyStartID;
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
