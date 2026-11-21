USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModPartyOwnerFlag]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-01-05
description : ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = ???? ????? SP??? ??? ????? ?? by ??? at 2012-03-20
**/

CREATE PROCEDURE [dbo].[P_ModPartyOwnerFlag]
	  @inbPartyID bigint --// ??ID
	, @inbCharacterID bigint --// ???? ? ??? ID
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
	/**_# ??? ??? ?????.*/
	BEGIN TRAN
		UPDATE dbo.PartyJoins 
		SET PartyOwnerFlag = 0
		WHERE PartyID = @inbPartyID AND PartyOwnerFlag = 1;

		UPDATE dbo.PartyJoins 
		SET PartyOwnerFlag = 1
		WHERE PartyID = @inbPartyID AND CharacterID = @inbCharacterID;
	COMMIT TRAN
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
