USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPartyJoinMembers]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-01-05
description : ??? ??? ???? ?????.
result set : version1 : CharacterID/bigint, CharacterName/nvarchar, CharacterLevel/tinyint, JobCode/tinyint, PartyOwnerFlag/bit
             version2 : CharacterID/bigint, CharacterName/nvarchar, CharacterLevel/tinyint, JobCode/tinyint, PartyOwnerFlag/bit, CharacterTypeCode/tinyint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
4 = PartyCode Output ???? ?? by ??? at 2012-03-26
5 = PartyCode ?? ?? by ??? at 2012-03-26
6 = ??? ?? by ??? at 2012-08-24
7 = ??? ???? ??? ?? ??? ?? by ??? at 2013-07-01
**/

CREATE PROCEDURE [dbo].[P_GetPartyJoinMembers]
	  @inbPartyID bigint --// ??ID
	, @inyPartyCode tinyint OUTPUT --// 0=????, 1=?????
	, @intVersion int = 1 --// ??? ??
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
	/**_# ????? ?????.*/
	SELECT @inyPartyCode = PartyCode 
	FROM dbo.Parties WITH(NOLOCK, INDEX(PK_Parties), FORCESEEK)
	WHERE PartyID = @inbPartyID;
	
	IF @intVersion = 1
		/**_# ?? ??? ?????.*/
		SELECT PJ.CharacterID, C.CharacterName, CS.CharacterLevel, CS.JobCode, PJ.PartyOwnerFlag
		FROM dbo.PartyJoins PJ WITH(NOLOCK, INDEX(PK_PartyJoins), FORCESEEK)
			INNER JOIN dbo.CharacterStatus CS WITH(NOLOCK, INDEX(PK_CharacterStatus), FORCESEEK) ON CS.CharacterID = PJ.CharacterID
			INNER JOIN dbo.Characters C WITH(NOLOCK, INDEX(PK_Characters), FORCESEEK) ON C.CharacterID = PJ.CharacterID
		WHERE  PartyID = @inbPartyID
		OPTION (FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);
	ELSE IF @intVersion = 2
		/**_# ?? ??? ?????.*/
		SELECT PJ.CharacterID, C.CharacterName, CS.CharacterLevel, CS.JobCode, PJ.PartyOwnerFlag, PJ.CharacterTypeCode
		FROM dbo.PartyJoins PJ WITH(NOLOCK, INDEX(PK_PartyJoins), FORCESEEK)
			INNER JOIN dbo.CharacterStatus CS WITH(NOLOCK, INDEX(PK_CharacterStatus), FORCESEEK) ON CS.CharacterID = PJ.CharacterID
			INNER JOIN dbo.Characters C WITH(NOLOCK, INDEX(PK_Characters), FORCESEEK) ON C.CharacterID = PJ.CharacterID
		WHERE  PartyID = @inbPartyID
		OPTION (FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);

	
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
