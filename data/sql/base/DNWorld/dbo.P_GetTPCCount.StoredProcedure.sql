USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetTPCCount]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-08-12
description : ?? ???? ?? T, P, C ?? ?????. (Teacher, Pupil, Classmate)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetTPCCount]
	  @inbMyCharacterID bigint --// ? ??? ID
	, @inbPartyCharacterID1 bigint --// ?? ??? ??? ID
	, @inbPartyCharacterID2 bigint --// ?? ??? ??? ID
	, @inbPartyCharacterID3 bigint --// ?? ??? ??? ID
	, @inbPartyCharacterID4 bigint --// ?? ??? ??? ID
	, @inbPartyCharacterID5 bigint --// ?? ??? ??? ID
	, @inbPartyCharacterID6 bigint --// ?? ??? ??? ID
	, @inbPartyCharacterID7 bigint --// ?? ??? ??? ID
	, @inbPartyCharacterID8 bigint --// ?? ??? ??? ID
	, @inbPartyCharacterID9 bigint --// ?? ??? ??? ID
	, @inbPartyCharacterID10 bigint --// ?? ??? ??? ID
	, @inbPartyCharacterID11 bigint --// ?? ??? ??? ID
	, @inbPartyCharacterID12 bigint --// ?? ??? ??? ID
	, @inbPartyCharacterID13 bigint --// ?? ??? ??? ID
	, @inbPartyCharacterID14 bigint --// ?? ??? ??? ID
	, @inbPartyCharacterID15 bigint --// ?? ??? ??? ID
	, @intTeacherCount int OUTPUT --// ?? ?
	, @intPupilCount int OUTPUT --// ?? ?
	, @intClassmateCount int OUTPUT --// ?? ?
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @tblCharacters table (CharacterID bigint NOT NULL PRIMARY KEY);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	INSERT @tblCharacters (CharacterID)
	SELECT @inbPartyCharacterID1 WHERE @inbPartyCharacterID1 IS NOT NULL UNION
	SELECT @inbPartyCharacterID2 WHERE @inbPartyCharacterID2 IS NOT NULL UNION
	SELECT @inbPartyCharacterID3 WHERE @inbPartyCharacterID3 IS NOT NULL UNION
	SELECT @inbPartyCharacterID4 WHERE @inbPartyCharacterID4 IS NOT NULL UNION
	SELECT @inbPartyCharacterID5 WHERE @inbPartyCharacterID5 IS NOT NULL UNION
	SELECT @inbPartyCharacterID6 WHERE @inbPartyCharacterID6 IS NOT NULL UNION
	SELECT @inbPartyCharacterID7 WHERE @inbPartyCharacterID7 IS NOT NULL UNION
	SELECT @inbPartyCharacterID8 WHERE @inbPartyCharacterID8 IS NOT NULL UNION
	SELECT @inbPartyCharacterID9 WHERE @inbPartyCharacterID9 IS NOT NULL UNION
	SELECT @inbPartyCharacterID10 WHERE @inbPartyCharacterID10 IS NOT NULL UNION
	SELECT @inbPartyCharacterID11 WHERE @inbPartyCharacterID11 IS NOT NULL UNION
	SELECT @inbPartyCharacterID12 WHERE @inbPartyCharacterID12 IS NOT NULL UNION
	SELECT @inbPartyCharacterID13 WHERE @inbPartyCharacterID13 IS NOT NULL UNION
	SELECT @inbPartyCharacterID14 WHERE @inbPartyCharacterID14 IS NOT NULL UNION
	SELECT @inbPartyCharacterID15 WHERE @inbPartyCharacterID15 IS NOT NULL;

	SELECT @intTeacherCount = COUNT(*)
	FROM dbo.TeachersAndPupils TnP WITH (NOLOCK)
		INNER JOIN @tblCharacters T ON TnP.TeacherCharacterID = T.CharacterID
	WHERE TnP.PupilCharacterID = @inbMyCharacterID AND TnP.GraduateFlag = CAST(0 AS bit);

	SELECT @intPupilCount = COUNT(*)
	FROM dbo.TeachersAndPupils TnP WITH (NOLOCK)
		INNER JOIN @tblCharacters T ON TnP.PupilCharacterID = T.CharacterID
	WHERE TnP.TeacherCharacterID = @inbMyCharacterID AND TnP.GraduateFlag = CAST(0 AS bit);

	SELECT @intClassmateCount = COUNT(*)
	FROM dbo.TeachersAndPupils TnP1 WITH (NOLOCK)
		INNER JOIN dbo.TeachersAndPupils TnP2 WITH (NOLOCK) ON TnP1.TeacherCharacterID = TnP2.TeacherCharacterID
		INNER JOIN @tblCharacters T ON TnP2.PupilCharacterID = T.CharacterID
	WHERE TnP1.PupilCharacterID = @inbMyCharacterID AND TnP1.GraduateFlag = CAST(0 AS bit) AND TnP2.GraduateFlag = CAST(0 AS bit);
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
