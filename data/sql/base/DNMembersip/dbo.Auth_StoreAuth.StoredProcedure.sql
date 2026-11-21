USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[Auth_StoreAuth]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Auth_StoreAuth]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[Auth_StoreAuth] AS' 
END
GO
/**
version : 11
author : ???
e-mail : b4nfter@eyedentitygames.com
created date : 2010-01-08
description : ???? ?? (??? ?? ??)
return value :
history :
6 = version 4? ?? at 2010-08-12
9~10 = Try..Catch.. ?? ??     by ??? at 2011-12-05
**/

ALTER PROCEDURE [dbo].[Auth_StoreAuth]
	 @pAccountDBID			INT				--// ?? DBID
	,@pCertifyingKey		BIGINT			--// ???
	,@pCurServerID			INT				--// ???? ?? ????
	,@pWorldID				TINYINT			--// ?? ID
	,@pSessionID			INT				--// ?? ID
	,@pCharacterDBID		BIGINT			--// ??? DBID
	,@pAccountName			NVARCHAR(50)	--// ?? ??
	,@pCharacterName		NVARCHAR(30)	--// ??? ??
	,@pIsAdult				BIT				--// ?? ??
	,@pAge					TINYINT			--// ??
	,@pDailyCreateCount		TINYINT			--// ?? ??? ???? ??
	,@pPrmInt1				INT				--// ???? 1 (??? ???)
	,@inyLastServerType tinyint = 0 --// ??? ?? ?? ID
AS
SET NOCOUNT ON
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @aCertifyingStep	TINYINT
DECLARE @aCurServerID		INT
/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @aCertifyingStep = 0;

	IF (@pCertifyingKey = 0)
	BEGIN
		SET  @intReturnValue = 1303;	-- ??? ?? ?? ? (????? ?? ?? ???? ?? ?)
		GOTO ErrorHandler;
	END

	-- ??? ??? ???? ????? ?? ???? ???? ?? ???? ?? ????? ??
	BEGIN TRANSACTION
	
	SELECT @aCertifyingStep = A.[CertifyingStep], @aCurServerID = A.[CurServerID]
	FROM dbo.DNAuth As A WITH(UPDLOCK)
	WHERE AccountDBID = @pAccountDBID;
	
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 1305;		-- ?????? ???? ??
		GOTO ErrorHandler;
	END

--	IF (@aCertifyingStep <> 2)
	IF (@aCertifyingStep NOT BETWEEN 1 AND 2)
	BEGIN
		SET  @intReturnValue = 1300;	-- ???? ?? ? (????? ?? ?? ???? ?? ?)
		GOTO ErrorHandler;
	END

	IF (@aCurServerID <> @pCurServerID)
	BEGIN
		SET  @intReturnValue = 1301;	-- ?? ???? ?? ? (????? ?? ?? ???? ?? ?)
		GOTO ErrorHandler;
	END

	UPDATE dbo.DNAuth
	SET 
		 CertifyingKey = @pCertifyingKey
		,CertifyingStep = 1
		,AccessTime = GETDATE()
		,WorldID = @pWorldID
		,SessionID = @pSessionID
		,CharacterDBID = @pCharacterDBID
		,AccountName = @pAccountName
		,CharacterName = @pCharacterName
		,IsAdult = @pIsAdult
		,Age = @pAge
		,DailyCreateCount = @pDailyCreateCount
		,PrmInt1 = @pPrmInt1
		,LastServerType = @inyLastServerType
	WHERE AccountDBID = @pAccountDBID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 1;
		GOTO ErrorHandler;
	END	
	
	COMMIT TRANSACTION;
	
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
