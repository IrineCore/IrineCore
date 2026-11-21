USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_UpsProfile]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-29
description : ??? ????? UPSERT ???.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_UpsProfile]
	  @inbCharacterID bigint --// ??? ID
	, @inyGenderCode tinyint --// 1=??, 2=??
	, @nvcSelfIntroduction nvarchar(50) --// ?? ??
	, @bitPublicFlag bit --// 0=???, 1=??
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
	/**_# [Profiles] ???? MERGE???.*/
	MERGE dbo.Profiles AS T
	USING (SELECT @inbCharacterID AS CharacterID, @inyGenderCode AS GenderCode, @nvcSelfIntroduction AS SelfIntroduction, @bitPublicFlag AS PublicFlag) AS S
	ON T.CharacterID = S.CharacterID
	WHEN MATCHED THEN
		UPDATE SET GenderCode = S.GenderCode, SelfIntroduction = S.SelfIntroduction, PublicFlag = S.PublicFlag
	WHEN NOT MATCHED BY TARGET THEN
		INSERT (CharacterID, GenderCode, SelfIntroduction, PublicFlag) VALUES (S.CharacterID, S.GenderCode, S.SelfIntroduction, S.PublicFlag);

	/**_# [TeacherCharacters] ???? UPDATE???.*/
	UPDATE dbo.TeacherCharacters
	SET GenderCode = @inyGenderCode
	WHERE TeacherCharacterID = @inbCharacterID;
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
