USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddParyJoin]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 9
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-01-05
description : ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103322 = ?? ??? ???? ????.
103323 = ?? ??? ??? ?????.
103330 = ?? ?? ?? ???? ???????.
history:
5 = ?????? ??, SELECT ??? NOLOCK?? ?? at 2012-06-20 by ??? 
**/

CREATE PROCEDURE [dbo].[P_AddParyJoin]
	  @inbPartyID bigint --// ??ID
	, @inbCharacterID bigint --// ???ID
	, @insPartyMaxCount smallint --// ?? ?? ???
	, @inyCharacterTypeCode tinyint = NULL--// 0=?????, 1=??????, 2=????, 3=??????
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

DECLARE @insCurrentPartyMemberCount smallint;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	BEGIN TRAN
		/**_# ?? ?? ???? ?? ???.*/
		SELECT @insCurrentPartyMemberCount = COUNT(*)
		FROM dbo.PartyJoins WITH(INDEX(PK_PartyJoins), FORCESEEK)
		WHERE PartyID = @inbPartyID
		OPTION(KEEPFIXED PLAN);

		IF @insCurrentPartyMemberCount >= @insPartyMaxCount
		BEGIN
			SET @intReturnValue = 103330;
			GOTO ErrorHandler;
		END
		
		/**_# [PartyJoins] ???? INSERT???.*/
		INSERT dbo.PartyJoins (PartyID, CharacterID, PartyOwnerFlag, CharacterTypeCode)
		VALUES (@inbPartyID, @inbCharacterID, CAST(0 AS bit), ISNULL(@inyCharacterTypeCode, 0));
	COMMIT TRAN
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%FK_Parties_PartyJoins_PartyID%'
		SET @intReturnValue = 103322;

	ELSE IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_PartyJoins%'
		SET @intReturnValue = 103323;

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
