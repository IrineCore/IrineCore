USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_Login]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_Login]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_Login] AS' 
END
GO
/**
version : 71
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-04
description : ??? ???.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
101101 = ?? ??? ??? ?????.
101102 = ??? ???? ????.
101108 = ??? ???? ?? ?????.
101109 = ??? ??? IP???.
101110 = ????? ???? ????.
101168 = ??? ? ?? IP? ??? ?????.
101194 = IP? ????? ?? ?? ???? ???????.
101223 = ?? ??? ?? ?????.
history :
36 = IP ?? ??? ????? ?? at 2010-07-07
37 = AccountLevelCode > 0? ??? ?? IP? ???? ?? ?? at 2010-08-18
38 = version 37? bug fix at 2010-08-25
39 = input param. @inyNationalityCode ?? at 2010-09-14
40 = input param. @inyChannelPartnerCode ?? at 2010-11-25
41 = input param. @inyAuthStep ?? at 2010-12-20
41 = ????? 0 ?? ???? ?? null ? ?? - 20110302.wgkim
46 = (44912) IP ? ?? ??? ?? ?? ?? ???? ?? ?? by ??? at 2011-11-09
47 = (44912) IP? ??? ? [ClientIPConnectCounts] ????? IP? ?????? ?? by ??? at 2011-11-16
48 = [BlockedIPExceptions] ???? ??? IP??, IP ? ?? ??? ?? ?? ?? ???? ??? ?? by ??? at 2011-12-06
49 = (48559) IP ?? ?? ?? by ??? at 2011-12-23
51 = (49590) IP ?? ?? ?? by ??? at 2012-01-13
52 = (49411) IPBlockMethodCode ??? ?? ?? ?? by ??? at 2012-01-25
53 = ver.52? ?? ?? ?? ?? by ??? at 2012-01-25
54 = (53590) output param. @dt2LastLogoutDate ?? by ??? at 2012-04-16
55 = ??? ?? ?? OUTPUT Param(@bitMobileAuthenticationUser) ?? by ??? at 2012-06-25
57 = @dt2LastLogoutDate ?? NULL? ? LastLoginDate? ????? ?? by ??? at 2012-07-04
58 = ClientIPConnectCounts ???? ?? by ??? at 2012-08-24
59 = (68598) output param. @inyCharacterSortCode ?? by ??? at 2012-09-19
64 = (75440) output param. @bitReturnRewardFlag ?? by ??? at 2012-12-04 
67 = (81067) ??? ?? ?? ?? by ??? at 2013-02-26
**/

ALTER PROCEDURE [dbo].[P_Login]
	  @nvcAccountName nvarchar(50) --// ??? ?? ??
	, @vchLoginIP varchar(15) --// ???? IP ??
	, @intSessionID int	--// ?? ???? ?? ID
	, @inyPublisherCode tinyint --// 0=????, 1=??, 2=??, 3=NHN Japan, 4=????, 5=?? ????, 6=SDGi
	, @inyCharacterCreateLimitDefault tinyint = 7 --// ?? ?? ??? ??? ? ???
	, @inyCharacterMaxCountDefault tinyint = 4 --// ?? ??? ? ?? ??? ? ???
	, @inyGenderCode tinyint = NULL --// (optional) 1=??, 2=??
	, @datBirthDate date = NULL --// (optional) ?? ?? (??? ??? ? ????.)
	, @vchPassphrase varchar(12) = NULL --// (optional) ?? ?? / ???? ?? ??
	, @bitNewAccount bit OUTPUT --// ?? Login???? ?? ??? ?????? ??. 0=??? ???? ?? ??, 1=?? ?? ??? ??
	, @intAccountID int OUTPUT --// ??? ?? ID
	, @inyAccountLevelCode tinyint OUTPUT --// 0=?? ??, 10=??(??), 20=????(??), 30=???(??), 99=QA, 100=???
	, @binGameOption binary(20) OUTPUT --// ?? ?? ?? (????)
	, @inyCharacterCreateLimit tinyint OUTPUT --// ?? ?? ?? ??? ??? ?
	, @inyCharacterMaxCount tinyint OUTPUT --// ?? ??? ? ?? ??? ?
	, @binMachineID binary(16) = NULL --// PC ???? ???
	, @inyNationalityCode tinyint = NULL --// ?? ??. 95	Hong Kong, 207=Taiwan
	, @inyChannelPartnerCode tinyint = NULL --// 1=MSN(Japan). ???? ?? ???? NULL?? ?????.
	, @inyAuthStep tinyint = NULL --// ?? ?? / 1=?? ?? ?? ??, 2=???? Authorization ?? ?? ? ??? ??
	, @intAccountKey int = NULL OUTPUT --// ???? ?? ? ?
	, @bitCheckConnectIPCount bit = 0 --// ????? IP? ?? ? ?? ??. 0=???? ??, 1=???
	, @dt2LastLogoutDate datetime2(0) = NULL OUTPUT --// ?? ???? ??
	, @bitMobileAuthenticationUser bit = NULL OUTPUT --// ??? ?? ?? ??
	, @inyCharacterSortCode tinyint = NULL OUTPUT --// 1=??, 2=??, 3=??, 4=??, 5=??, 6=?? ??
	, @bitNewbieRewardFlag bit = NULL OUTPUT --// 0=?????? ?? ??, 1=?????? ??
	, @dt2RegisterDate datetime2(0) = NULL OUTPUT --// ?? ????
	, @bitReturnRewardFlag bit = NULL OUTPUT --// 0=??????? ?? ??, 1=??????? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @intLoginIP int = dbo.FN_IP2IPNumber(@vchLoginIP)
	, @sdtBlockStartDate smalldatetime
	, @sdtBlockEndDate smalldatetime
	, @intTemporaryBlockID int
	, @binPassphrase binary(20)
	, @intTemp int
	, @inbPetalAmount bigint
	, @inbPetalIncomeID bigint
	, @inyConnectCount tinyint
	, @bitConnectNoticeFlag bit
	, @intEventConditionID int
	, @intFirstPetalAmount int
	, @intPetalIncomeCountLimit int
	, @intConsecutionBonusCount int
	, @intConsecutionBonusPetalAmount int
	, @intEventScheduleID int
	, @intPetalAmount int
	, @intPetalIncomeCount int
	, @bitLockFlag bit;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
    /**_# ????? 0 ?? ???? ?? null ? ?????. */
    SET @inyNationalityCode = NULLIF(ISNULL(@inyNationalityCode, 0), 0);
    
    SET @inyPublisherCode = 0; /* any error, so set it = 0 2013.10.28*/
  
	IF LTRIM(RTRIM(@nvcAccountName)) = N''
	BEGIN
		SET @intReturnValue = 101102;
		GOTO ErrorHandler;
	END

	/**_# ??? ??? IP?? 101109? ?????.*/
	IF EXISTS (
		SELECT * 
		FROM dbo.BlockedIPs WITH (NOLOCK) 
		WHERE @intLoginIP BETWEEN StartIP AND EndIP 
			AND (StartDate IS NULL OR GETDATE() BETWEEN StartDate AND EndDate)
			AND IPBlockMethodCode = 1
	)
	BEGIN
		SET @intReturnValue = 101109;
		GOTO ErrorHandler;
	END

	/**_# ??? ??? ?? & ?? ?? IP? ??? 101108? ?????.*/
	SELECT @intTemporaryBlockID = TemporaryBlockID, @sdtBlockStartDate = StartDate, @sdtBlockEndDate = EndDate
	FROM dbo.TemporaryBlock WITH (NOLOCK)
	WHERE DeleteFlag = CAST(0 AS bit);

	IF @@ROWCOUNT > 0
		AND @dt2Now BETWEEN @sdtBlockStartDate AND @sdtBlockEndDate
		AND NOT EXISTS (
			SELECT * 
			FROM dbo.TemporaryOpenedIPs WITH (NOLOCK) 
			WHERE TemporaryBlockID = @intTemporaryBlockID AND IP = @intLoginIP
		)
	BEGIN
		SET @intReturnValue = 101108;
		GOTO ErrorHandler;
	END

	/**_# ?? ??? ?????.*/
	SELECT @intAccountID = AC.AccountID
		, @inyAccountLevelCode = AC.AccountLevelCode
		, @binGameOption = AC.GameOption
		, @inyCharacterCreateLimit = CASE WHEN AC.LastCharacterCreateDate IS NULL OR AC. LastCharacterCreateDate < CAST(@dt2Now AS date) THEN @inyCharacterCreateLimitDefault ELSE AC.CharacterCreateLimit END
		, @inyCharacterMaxCount = AC.CharacterMaxCount
		, @binPassphrase = AC.Passphrase
		, @intAccountKey = AC.AccountKey	
		, @dt2LastLogoutDate = COALESCE(AC.LastLogoutDate, AC.LastLoginDate, GETDATE())
		, @bitMobileAuthenticationUser = CASE WHEN (MA.AccountID IS NOT NULL AND MA.AccountAuthenticationKey IS NULL) THEN 1 ELSE 0 END
		, @inyCharacterSortCode = ISNULL(CharacterSortCode, 6)
		, @bitConnectNoticeFlag = ISNULL(ConnectNoticeFlag, CAST(0 AS bit))
		, @bitNewbieRewardFlag = ISNULL(NewbieRewardFlag, CAST(1 AS bit))
		, @dt2RegisterDate = RegisterDate
		, @bitReturnRewardFlag = ISNULL(ReturnRewardFlag, CAST(0 AS bit))
		, @bitLockFlag = ISNULL(LockFlag, CAST(0 AS bit))
	FROM dbo.Accounts AC WITH (NOLOCK, INDEX(IX_UN_Accounts_AccountName), FORCESEEK) 
         LEFT OUTER JOIN dbo.MobileAuthentication MA WITH (NOLOCK) ON AC.AccountID = MA.AccountID
	WHERE AC.AccountName = @nvcAccountName;

	SET @intTemp = @@ROWCOUNT;

	/**_# ?? ??? ?? 101223? ?????.*/
	IF @bitLockFlag = 1
	BEGIN
		SET @intReturnValue = 101223;
		GOTO ErrorHandler;
	END	

	/**_# ??? ?? ??? ?? ?? & GMAccountAccessIPs ???? ?? ?? ?? & GMAccountAccessIPs? ?? ??? IP? ???? ?? ??? 101168? ?????.*/
	IF @inyAccountLevelCode > 0
		AND EXISTS (SELECT * FROM dbo.GMAccountAccessIPs WITH (NOLOCK))
		AND NOT EXISTS (SELECT * FROM dbo.GMAccountAccessIPs WITH (NOLOCK) WHERE IP = dbo.FN_IP2IPNumber(@vchLoginIP))
	BEGIN
		SET @intReturnValue = 101168;
		GOTO ErrorHandler;
	END

	/**_# ??? ???? ???...*/
	IF @intTemp = 0
	BEGIN
		/**_## ?? ??? ?? 101102? ?????. (?? ?? ??? ?? ????? [P_AddAccount]? ?? ???? ??? ?????.)*/
		IF @inyPublisherCode = 0
		BEGIN
			SET @intReturnValue = 101102;
			GOTO ErrorHandler;
		END

		/**_## ?? ??? ??? [P_AddAccount]? ???? ??? ?????.*/
		ELSE
		BEGIN
			EXEC @intReturnValue = dbo.P_AddAccount
				  @nvcAccountName = @nvcAccountName
				, @inyAccountLevelCode = 0
				, @inyCharacterCreateLimit = @inyCharacterCreateLimitDefault
				, @inyCharacterMaxCount = @inyCharacterMaxCountDefault
				, @dt2LastLoginDate = @dt2Now
				, @vchLastLoginIP = @vchLoginIP
				, @intLastSessionID = @intSessionID
				, @vchJoinIP = @vchLoginIP
				, @inyPublisherCode = @inyPublisherCode
				, @inyGenderCode = @inyGenderCode
				, @datBirthDate = @datBirthDate
				, @inyNationalityCode = @inyNationalityCode
				, @inyChannelPartnerCode = @inyChannelPartnerCode
				, @intAccountID = @intAccountID OUTPUT;

			IF @intReturnValue <> 0
				GOTO ErrorHandler;

			SET @bitNewAccount = 1;
			SET @inyAccountLevelCode = 0;
			SET @inyCharacterCreateLimit = @inyCharacterCreateLimitDefault;
			SET @inyCharacterMaxCount = @inyCharacterMaxCountDefault;
			SET @dt2LastLogoutDate = GETDATE();
		END
	END

	ELSE
	BEGIN
		/**_# ??? ??? ???? ??? ??? IP? ??? 101109? ?????.*/
		IF EXISTS (SELECT * FROM dbo.TestAccounts WITH (NOLOCK) WHERE AccountID = @intAccountID)
			AND NOT EXISTS (SELECT * FROM dbo.TestAccounts WITH (NOLOCK) WHERE AccountID = @intAccountID AND IP = @intLoginIP)
		BEGIN
			SET @intReturnValue = 101109;
			GOTO ErrorHandler;
		END

		/**_# ?? ???? ????? ???? ??? 101110? ?????.*/
		IF (@inyPublisherCode = 0) AND (@binPassphrase <> dbo.FN_HashPassphrase2(@vchPassphrase, @intAccountID))
		BEGIN
			SET @intReturnValue = 101110;
			GOTO ErrorHandler;
		END

		SET @bitNewAccount = 0;

		/**_# ??? ?? ??? ??? UPDATE ???.*/
		IF @inyAuthStep IS NULL OR @inyAuthStep = 2
			UPDATE dbo.Accounts
			SET LastLoginDate = @dt2Now, LastLoginIP = dbo.FN_IP2IPNumber(@vchLoginIP), LastSessionID = @intSessionID
			WHERE AccountID = @intAccountID;
	END

	SELECT @dt2RegisterDate = RegisterDate
		 , @bitNewbieRewardFlag = ISNULL(NewbieRewardFlag, CAST(1 AS bit))
		 , @bitReturnRewardFlag = ISNULL(ReturnRewardFlag, CAST(0 AS bit))
	FROM dbo.Accounts AC WITH (NOLOCK, INDEX(IX_UN_Accounts_AccountName), FORCESEEK) 
         LEFT OUTER JOIN dbo.MobileAuthentication MA WITH (NOLOCK) ON AC.AccountID = MA.AccountID
	WHERE AC.AccountName = @nvcAccountName;

	/**_# ?? ??? ??? ?????.*/
	IF @inyAuthStep IS NULL OR @inyAuthStep = 2
		EXEC dbo.P_AddAccountLoginLog
			  @dt2LoginDate = @dt2Now
			, @intAccountID = @intAccountID
			, @intLoginIP = @intLoginIP
			, @intSessionID = @intSessionID
			, @binMachineID = @binMachineID;

	/**_# ????? IP? ?? ?? ?????.*/
	IF @bitCheckConnectIPCount = 1 AND NOT EXISTS (
		SELECT *
		FROM dbo.BlockedIPExceptions WITH (NOLOCK)
		WHERE @intLoginIP BETWEEN StartIP AND EndIP
	)
	BEGIN
		IF NOT EXISTS(SELECT * FROM dbo.ClientIPConnectCounts WITH(NOLOCK) WHERE IP = @intLoginIP AND AccountID = @intAccountID)
			INSERT dbo.ClientIPConnectCounts (IP, AccountID)
			VALUES (@intLoginIP, @intAccountID);

		SELECT @inyConnectCount = ConnectCount
		FROM dbo.VIX_ClientIPConnectCounts WITH (NOLOCK, NOEXPAND, FORCESEEK)
		WHERE IP = @intLoginIP
		OPTION (KEEPFIXED PLAN);

		IF @inyConnectCount >= (SELECT SimpleConfigValue FROM dbo.SimpleConfigValues WITH (NOLOCK) WHERE SimpleConfigCode = 1)
		BEGIN
			EXEC @intReturnValue = dbo.P_AddBlockedIP
				  @vchStartIP = @vchLoginIP
				, @vchEndIP = @vchLoginIP
				, @inyBlockedIPCode = 255
				, @nvcDescription = N''
				, @intAdminID = 0
				, @sdtStartDate = NULL
				, @sdtEndDate = NULL;

			IF @intReturnValue = 0
			BEGIN
				DELETE dbo.ClientIPConnectCounts
				WHERE IP = @intLoginIP;

				SET @intReturnValue = 101194;
				GOTO ErrorHandler;
			END
			ELSE
				SET @intReturnValue = 0
		END
	END

	/**_# ??? ?? ?? Type 1*/
	SELECT @inbPetalAmount = PetalAmount
	FROM dbo.EventPetalForCBTUsers WITH (NOLOCK, INDEX(PK_EventPetalForCBTUsers), FORCESEEK)
	WHERE AccountName = @nvcAccountName AND CompleteFlag = CAST(0 AS bit)
	OPTION (KEEPFIXED PLAN);

	IF @@ROWCOUNT > 0
	BEGIN
		BEGIN TRANSACTION;
		INSERT dbo.PetalIncome (AccountID, PetalIncomeCode, PurchaseOrderID, AuditLogID, PetalAmount, IncomeDate)
		VALUES (@intAccountID, 4, NULL, NULL, @inbPetalAmount, @dt2Now);

		SET @inbPetalIncomeID = SCOPE_IDENTITY();

		UPDATE dbo.EventPetalForCBTUsers
		SET CompleteFlag = 1, PetalIncomeID = @inbPetalIncomeID
		WHERE AccountName = @nvcAccountName;

		COMMIT TRANSACTION;
	END

	/**_# ??? ?? ?? Type 2*/
	SELECT @intEventScheduleID = EventScheduleID
		, @intPetalAmount = PetalAmount
		, @intEventConditionID = EventConditionID
	FROM dbo.EventScheduleForPetal WITH (NOLOCK)
	WHERE CAST(CONVERT(char(16), @dt2Now, 121) AS smalldatetime) BETWEEN StartDate AND EndDate;

	IF @@ROWCOUNT > 0 AND NOT EXISTS (
		SELECT *
		FROM dbo.EventScheduleDetailForPetal A WITH (NOLOCK, INDEX(PK_EventScheduleDetailForPetal), FORCESEEK)
		WHERE A.AccountID = @intAccountID
			AND A.EventScheduleID = @intEventScheduleID
	)
	BEGIN
		IF @intEventConditionID IS NOT NULL
		BEGIN
			SELECT @intFirstPetalAmount = FirstPetalAmount
				, @intPetalIncomeCountLimit = PetalIncomeCountLimit
				, @intConsecutionBonusCount = ConsecutionBonusCount
				, @intConsecutionBonusPetalAmount = ConsecutionBonusPetalAmount
			FROM dbo.EventConditionForPetal WITH (NOLOCK)
			WHERE EventConditionID = @intEventConditionID;

			SELECT @intPetalIncomeCount = Cnt 
			FROM dbo.VIX_EventScheduleDetailForPetal WITH (NOLOCK, NOEXPAND) 
			WHERE AccountID = @intAccountID 
				AND EventConditionID = @intEventConditionID;

			IF @@ROWCOUNT = 0
				SET @intPetalIncomeCount = 0;

			SET @intPetalAmount = 
				CASE
					WHEN @intPetalIncomeCount = 0 THEN @intFirstPetalAmount
					WHEN @intPetalIncomeCount >= @intPetalIncomeCountLimit THEN 0
					ELSE @intPetalAmount
				END;

			SET @intConsecutionBonusPetalAmount =
				CASE
					WHEN EXISTS (
							SELECT * 
							FROM dbo.EventScheduleForPetal ES WITH (NOLOCK)
								INNER JOIN dbo.EventScheduleDetailForPetal ESD WITH (NOLOCK) ON ES.EventScheduleID = ESD.EventScheduleID
							WHERE ES.EventConditionID = @intEventConditionID
								AND ESD.AccountID = @intAccountID
								AND ESD.ConsecutionBonusPetalIncomeID IS NOT NULL
						) THEN 0
					WHEN @intConsecutionBonusCount IS NOT NULL 
						AND (
							SELECT COUNT(*) 
							FROM dbo.EventScheduleForPetal ES WITH (NOLOCK)
								INNER JOIN dbo.EventScheduleDetailForPetal ESD WITH (NOLOCK) ON ES.EventScheduleID = ESD.EventScheduleID
							WHERE ES.EventConditionID = @intEventConditionID
								AND ESD.AccountID = @intAccountID
								AND ESD.RegisterDate >= CAST(CAST(DATEADD(DAY, -(@intConsecutionBonusCount - 1), @dt2Now) AS date) AS datetime2(0))
						) = (@intConsecutionBonusCount - 1) THEN @intConsecutionBonusPetalAmount
					ELSE 0
				END;
		END

		BEGIN TRANSACTION;

		IF @intPetalAmount > 0
		BEGIN
			INSERT dbo.PetalIncome (AccountID, PetalIncomeCode, PurchaseOrderID, AuditLogID, PetalAmount, IncomeDate)
			VALUES (@intAccountID, 4, NULL, NULL, @intPetalAmount, @dt2Now);

			SET @inbPetalIncomeID = SCOPE_IDENTITY();
		END

		INSERT dbo.EventScheduleDetailForPetal (EventScheduleID, AccountID, PetalIncomeID, RegisterDate, ConsecutionBonusPetalIncomeID)
		VALUES (@intEventScheduleID, @intAccountID, CASE WHEN @intPetalAmount > 0 THEN @inbPetalIncomeID ELSE NULL END, @dt2Now, NULL);

		IF @intConsecutionBonusPetalAmount > 0
		BEGIN
			INSERT dbo.PetalIncome (AccountID, PetalIncomeCode, PurchaseOrderID, AuditLogID, PetalAmount, IncomeDate)
			VALUES (@intAccountID, 4, NULL, NULL, @intConsecutionBonusPetalAmount, @dt2Now);

			SET @inbPetalIncomeID = SCOPE_IDENTITY();

			UPDATE dbo.EventScheduleDetailForPetal
			SET ConsecutionBonusPetalIncomeID = @inbPetalIncomeID
			WHERE EventScheduleID = @intEventScheduleID
				AND AccountID = @intAccountID;
		END

		COMMIT TRANSACTION;
	END

	/**_# ?? ?? ?? ???? ?? ?? ?? ?? ?? ?? INSERT */
	IF @bitConnectNoticeFlag = CAST(1 AS bit)
		INSERT dbo.AccountConnectNoticeQueue (AccountID, RegistedDate) VALUES (@intAccountID, @dt2Now);

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
