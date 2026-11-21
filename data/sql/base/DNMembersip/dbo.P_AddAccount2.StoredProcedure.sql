USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_AddAccount2]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_AddAccount2]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_AddAccount2] AS' 
END
GO
ALTER PROCEDURE [dbo].[P_AddAccount2]
	  @nvcAccountName nvarchar(50) 
	, @inyAccountLevelCode tinyint 
	, @inyCharacterCreateLimit tinyint
	, @inyCharacterMaxCount tinyint 
	, @dt2LastLoginDate datetime2(0)= NULL 
	, @vchLastLoginIP varchar(15) = NULL
	, @intLastSessionID int = NULL 
	, @vchJoinIP varchar(15) = NULL 
	, @inyPublisherCode tinyint 
	, @inyGenderCode tinyint = NULL 
	, @datBirthDate date = NULL 
	, @vchPassphrase varchar(12) = NULL
	, @inyNationalityCode tinyint = NULL 
	, @inyChannelPartnerCode tinyint = NULL
	, @intAccountID int = NULL OUTPUT 
-- WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;
DECLARE @intReturnValue int;


IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	INSERT dbo.Accounts (
	AccountName,
	 AccountLevelCode,
	  GameOption, 
	  CharacterCreateLimit,
	   LastCharacterCreateDate, 
	   CharacterMaxCount,
	    LastLoginDate, LastLogoutDate, 
	    LastLoginIP, LastSessionID, JoinIP, RegisterDate, PublisherCode, GenderCode, BirthDate,
	     Passphrase, NationalityCode, 
	     ChannelPartnerCode,
	     [AccountKey],
	     [CharacterSortCode],
	     [ConnectNoticeFlag],
	     [NewbieRewardFlag],
	     [NewbieRewardDate],
         [ReturnRewardFlag],
         [LockFlag]
	  
	     )
	VALUES (@nvcAccountName, @inyAccountLevelCode, NULL, @inyCharacterCreateLimit, NULL, @inyCharacterMaxCount, @dt2LastLoginDate, NULL, dbo.FN_IP2IPNumber(@vchLastLoginIP), @intLastSessionID, NULL, GETDATE(), @inyPublisherCode, @inyGenderCode, @datBirthDate, CASE @inyPublisherCode WHEN 0 THEN 0x0 ELSE NULL END, @inyNationalityCode
	, @inyChannelPartnerCode,
	Null,
	6,
	Null,
	1,
	GETDATE(),
	Null,
	Null
	);

	SET @intAccountID = SCOPE_IDENTITY();


	IF @inyPublisherCode = 0 AND @vchPassphrase IS NOT NULL
		UPDATE dbo.Accounts
		SET Passphrase = dbo.FN_HashPassphrase2(@vchPassphrase, @intAccountID)
		WHERE AccountID = @intAccountID;
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2601 AND ERROR_MESSAGE() LIKE N'%IX_UN_Accounts_AccountName%'
		SET @intReturnValue = 101101;

	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

RETURN @intReturnValue;
GO
