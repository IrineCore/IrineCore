USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_StoreTargetCharacter]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_StoreTargetCharacter]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_StoreTargetCharacter] AS' 
END
GO
/**
version : 6
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-08-27
description : DOLIS? ?? ?? ??? ?? ??? ID? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101170 = @vchCertKey? ?? ????.
101171 = ?????? ?? ?? ?? ????.
101185 = CharacterID? ??? ?????.
history :
2 = ?? ? ?? ?? ?? at 2010-08-27
3 = CharacterID? ???? ?? ???? ??? ?? at 2010-11-19
4 = TargetCharacterDetails ???? Truncate ?? Delete?? ???? ?? at 2011-01-06
5 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-02
6 = Option / Index Hint ?? ? Join ?? ??     by ??? at 2012-03-16
**/

ALTER PROCEDURE [dbo].[P_StoreTargetCharacter]
	  @inyTargetType tinyint --// 1=??, 2=??, 3=??? ??
	, @intWorldID int --// ?? ID
	, @vchCharacterIDs varchar(max) --// ?? ??? ID ??? (?? 1000?). ???? ,???. ???? ???? ???? NULL ?????.
	, @nvcCharacterNames nvarchar(max) --// ?? ??? ?? ??? (?? 500?). ???? ,???. ???? ???? ???? NULL ?????.
	, @vchCertKey char(32) = NULL OUTPUT --// cert. key
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @nvcStmt nvarchar(max), @dtmRegisterDate datetime;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF OBJECT_ID(N'TargetCharacters', N'U') IS NULL
		CREATE TABLE dbo.TargetCharacters (
			CertKey char(32) NOT NULL,
			RegisterDate datetime NOT NULL DEFAULT(GETDATE())
		);

	IF OBJECT_ID(N'TargetCharacterDetails', N'U') IS NULL
		CREATE TABLE dbo.TargetCharacterDetails (
			Seq int IDENTITY(1, 1) NOT NULL PRIMARY KEY,
			CharacterID bigint NOT NULL,
			CompleteFlag bit NOT NULL DEFAULT(0)
		);

	IF NOT EXISTS (SELECT * FROM sys.indexes WHERE name = N'IX_UN_TargetCharacterDetails_CharacterID')
		CREATE UNIQUE INDEX IX_UN_TargetCharacterDetails_CharacterID ON dbo.TargetCharacterDetails (CharacterID) ON IndexFG1;

	IF OBJECT_ID(N'VIX_TargetCharacterDetails', N'V') IS NULL
	BEGIN
		SET @nvcStmt = N'
CREATE VIEW dbo.VIX_TargetCharacterDetails
WITH SCHEMABINDING
AS
SELECT CompleteFlag, COUNT_BIG(*) AS Cnt
FROM dbo.TargetCharacterDetails
GROUP BY CompleteFlag'
		EXEC (@nvcStmt);

		SET @nvcStmt = N'CREATE UNIQUE CLUSTERED INDEX IX_UC_VIX_TargetCharacterDetails ON dbo.VIX_TargetCharacterDetails (CompleteFlag) ON IndexFG1;'

		EXEC (@nvcStmt);
	END		

	IF @vchCertKey IS NOT NULL
	BEGIN
		IF NOT EXISTS (SELECT * FROM dbo.TargetCharacters WHERE CertKey = @vchCertKey)
		BEGIN
			SET @intReturnValue = 101170;
			GOTO ErrorHandler;
		END
	END

	ELSE
	BEGIN
		SELECT @dtmRegisterDate = RegisterDate FROM dbo.TargetCharacters;

		IF @dtmRegisterDate IS NOT NULL
		BEGIN
			IF DATEDIFF(MINUTE, @dtmRegisterDate, GETDATE()) < 30
			BEGIN
				SET @intReturnValue = 101171;
				GOTO ErrorHandler;
			END

			ELSE
			BEGIN
				TRUNCATE TABLE dbo.TargetCharacters;
				DELETE dbo.TargetCharacterDetails;

				SET @vchCertKey = REPLACE(NEWID(), '-', '');
				INSERT dbo.TargetCharacters (CertKey) VALUES (@vchCertKey);
			END
		END

		ELSE
		BEGIN
			SET @vchCertKey = REPLACE(NEWID(), '-', '');
			INSERT dbo.TargetCharacters (CertKey) VALUES (@vchCertKey);
		END
	END

	IF @inyTargetType = 1
		INSERT dbo.TargetCharacterDetails (CharacterID)
		SELECT CharacterID
		FROM dbo.Characters WITH (NOLOCK)
		WHERE DeleteFlag = CAST(0 AS bit);

	ELSE IF @inyTargetType = 2
		INSERT dbo.TargetCharacterDetails (CharacterID)
		SELECT CharacterID
		FROM dbo.Characters WITH (NOLOCK)
		WHERE WorldID = @intWorldID AND DeleteFlag = CAST(0 AS bit);

	ELSE IF @inyTargetType = 3
		INSERT dbo.TargetCharacterDetails (CharacterID)
		SELECT A.CharacterID
		FROM dbo.Characters A WITH (NOLOCK, FORCESEEK)
			INNER JOIN dbo.TF_Split(@vchCharacterIDs, ',') F ON A.CharacterID = F.string
		WHERE A.DeleteFlag = CAST(0 AS bit)
		UNION ALL
		SELECT A.CharacterID
		FROM dbo.TF_NSplit(@nvcCharacterNames, N',') F
			INNER JOIN dbo.Characters A WITH (NOLOCK, FORCESEEK, INDEX(IX_UNF_Characters_CharacterName)) ON A.CharacterName = F.string
		WHERE A.DeleteFlag = CAST(0 AS bit)
		AND A.CharacterName IS NOT NULL
		OPTION (FORCE ORDER, LOOP JOIN, KEEPFIXED PLAN);

END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2601 AND ERROR_MESSAGE() LIKE N'%IX_UN_TargetCharacterDetails_CharacterID%'
	BEGIN
		DROP VIEW dbo.VIX_TargetCharacterDetails;
		DROP TABLE dbo.TargetCharacters;
		DROP TABLE dbo.TargetCharacterDetails;

		SET @intReturnValue = 101185;
	END

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
