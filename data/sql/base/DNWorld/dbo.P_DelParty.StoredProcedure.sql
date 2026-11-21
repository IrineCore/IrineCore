USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DelParty]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-01-05
description : ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
7 = ?? ?? ?? by ??? at 2012-05-14
**/

CREATE PROCEDURE [dbo].[P_DelParty]
	  @inbPartyID bigint --// ??ID
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
	BEGIN TRANSACTION;
	/**_# ?? Party? ?? ???? Delete???.*/
	DELETE dbo.PartyJoins
	FROM dbo.PartyJoins WITH(INDEX(PK_PartyJoins), FORCESEEK)
	WHERE PartyID = @inbPartyID
	OPTION (KEEPFIXED PLAN);

	/**_# ?? Party? Delete???.*/
	DELETE dbo.Parties 
	FROM dbo.Parties WITH(INDEX(PK_Parties), FORCESEEK)
	WHERE PartyID = @inbPartyID
	OPTION (KEEPFIXED PLAN);
	
	COMMIT TRANSACTION;

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
