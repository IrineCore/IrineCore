USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_AddEventReward]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DLS_AddEventReward]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DLS_AddEventReward] AS' 
END
GO
/**
version : 8
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2012-08-23

description :
	??? ?? ??
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	101205 = ??? ?? ?? ?? ??? ????.
	101206 = ??? ?? ?? ??? ??? ????.
	101204 = ??? ?? ?? ??? ????.
**/

ALTER PROCEDURE [dbo].[P_DLS_AddEventReward]
	  @inyReceiveTypeCode tinyint --// ?? ?? (1=????, 2=????)
	, @inyTargetTypeCode tinyint --// ?? ?? (1=????, 2=????, 3=?????, 4=?????, 5=????)
	, @xmlAccountIDs xml = NULL --// (@inyTargetTypeCode=2 ? ?? ??) ?? ID ??   <root><Account AccountID="" /></root>
	, @xmlAccountNames xml = NULL --// (@inyTargetTypeCode=2 ? ?? ??) ?? ?? ??   <root><Account AccountName="" /></root>
	, @xmlCharacterIDs xml = NULL --// (@inyTargetTypeCode=4 ? ?? ??) ??? ID ??   <root><Character CharacterID="" /></root>
	, @xmlCharacterNames xml = NULL --// (@inyTargetTypeCode=4 ? ?? ??) ??? ?? ??   <root><Character CharacterName="" /></root>
	, @inyTargetWorldID tinyint = NULL --// (@inyTargetTypeCode<>1 ? ?? ??) ?? ID
	, @inyTargetClassCode tinyint = NULL --// ???? (1=???, 2=??, 3=????, 4=???, 5=????, 6=??)
	, @inyTargetMinLevel tinyint = NULL --// ?? ??
	, @inyTargetMaxLevel tinyint = NULL --// ?? ??
	, @sdtReserveSendDate smalldatetime --// ???? ??
	, @sdtExpirationDate smalldatetime --// ?? ??
	, @nvcEventName nvarchar(50) = NULL --// ??? ??
	, @nvcSenderName nvarchar(50) --// ??? ?? ??
	, @nvcContent nvarchar(200) --// ??
	, @inbRewardCoin bigint --// ?? ????
	, @xmlEventRewardItems xml = NULL --// ??? ?? ??? ??   <root><Item ProductFlag="" ItemID="" ItemCount="" ItemDurability="" RandomSeed="" ItemLevel="" ItemPotential="" SoulBoundFlag="" SealCount="" ItemOption="" ItemLifespan="" EternityFlag="" DragonJewelType=""  /></root>
	, @inyEventRewardTypeCode tinyint --// 1=??? ??, 2=??? ??? ??
	, @intEventRewardID int = NULL OUTPUT --// ??? ?? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @sdtNow smalldatetime = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	-- @inyTargetTypeCode ?? (2=????) ??? ?? ??? ?? ?? ??
	IF @inyTargetTypeCode = 2 AND @xmlAccountIDs IS NULL AND @xmlAccountNames IS NULL
	BEGIN
		SET @intReturnValue = 101205;
		GOTO ErrorHandler;
	END

	-- @inyTargetTypeCode ?? (4=?????) ??? ??? ??? ?? ?? ??
	IF @inyTargetTypeCode = 4 AND @xmlCharacterIDs IS NULL AND @xmlCharacterNames IS NULL
	BEGIN
		SET @intReturnValue = 101206;
		GOTO ErrorHandler;
	END

	-- @inyTargetTypeCode ?? (5=????) ??? ?? ??? ?? ?? ??
	IF @inyTargetTypeCode = 5 AND @inyTargetWorldID IS NULL AND @inyTargetClassCode IS NULL AND @inyTargetMinLevel IS NULL AND @inyTargetMaxLevel IS NULL
	BEGIN
		SET @intReturnValue = 101204;
		GOTO ErrorHandler;
	END

	/**_# ??? ?? ?? */
	INSERT dbo.EventRewards (ReceiveTypeCode, TargetTypeCode, TargetWorldID, TargetClassCode, TargetMinLevel, TargetMaxLevel, DeleteFlag, RegistrationDate, ReserveSendDate, ExpirationDate, EventName, SenderName, Content, RewardCoin, SystemSendFlag, EventRewardTypeCode)
	SELECT @inyReceiveTypeCode
		 , @inyTargetTypeCode
		 , CASE WHEN @inyTargetTypeCode <> 1 THEN @inyTargetWorldID ELSE NULL END
		 , @inyTargetClassCode
		 , @inyTargetMinLevel
		 , @inyTargetMaxLevel
		 , CONVERT(bit, 0)
		 , @sdtNow
		 , @sdtReserveSendDate
		 , @sdtExpirationDate
		 , @nvcEventName
		 , @nvcSenderName
		 , @nvcContent
		 , @inbRewardCoin
		 , CONVERT(bit, 0)
		 , @inyEventRewardTypeCode;

	SET @intEventRewardID = SCOPE_IDENTITY();
	/**_# ??? ?? ??? ?? */
	IF @xmlEventRewardItems IS NOT NULL
	BEGIN
		WITH T AS (
			SELECT T.col.value('@ProductFlag', 'bit') AS ProductFlag
				 , T.col.value('@ItemID', 'bigint') AS ItemID
				 , T.col.value('@ItemCount', 'smallint') AS ItemCount
				 , T.col.value('@ItemDurability', 'smallint') AS ItemDurability
				 , T.col.value('@RandomSeed', 'int') AS RandomSeed
				 , T.col.value('@ItemLevel', 'tinyint') AS ItemLevel
				 , T.col.value('@ItemPotential', 'tinyint') AS ItemPotential
				 , T.col.value('@SoulBoundFlag', 'bit') AS SoulBoundFlag
				 , T.col.value('@SealCount', 'tinyint') AS SealCount
				 , T.col.value('@ItemOption', 'tinyint') AS ItemOption
				 , T.col.value('@ItemLifespan', 'int') AS ItemLifespan
				 , T.col.value('@EternityFlag', 'bit') AS EternityFlag
				 , DATEADD(MINUTE, CASE T.col.value('@EternityFlag', 'bit') WHEN 0 THEN T.col.value('@ItemLifespan', 'int') ELSE 525600 END, @sdtNow) AS ItemExpireDate
				 , T.col.value('@DragonJewelType', 'tinyint') AS DragonJewelType
			  FROM @xmlEventRewardItems.nodes('/root/Item') T(col)
		)
		INSERT dbo.EventRewardItems (EventRewardID, ProductFlag, ItemID, ItemCount, ItemDurability, RandomSeed, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemLifespan, EternityFlag, ItemExpireDate, DragonJewelType)
		SELECT @intEventRewardID
			 , ProductFlag
			 , ItemID
			 , CASE WHEN ProductFlag = CONVERT(bit, 1) THEN NULL ELSE ItemCount END
			 , CASE WHEN ProductFlag = CONVERT(bit, 1) THEN NULL ELSE ItemDurability END
			 , CASE WHEN ProductFlag = CONVERT(bit, 1) THEN NULL ELSE RandomSeed END
			 , CASE WHEN ProductFlag = CONVERT(bit, 1) THEN NULL ELSE ItemLevel END
			 , CASE WHEN ProductFlag = CONVERT(bit, 1) THEN NULL ELSE ItemPotential END
			 , CASE WHEN ProductFlag = CONVERT(bit, 1) THEN NULL ELSE SoulBoundFlag END
			 , CASE WHEN ProductFlag = CONVERT(bit, 1) THEN NULL ELSE SealCount END
			 , CASE WHEN ProductFlag = CONVERT(bit, 1) THEN NULL ELSE ItemOption END
			 , CASE WHEN ProductFlag = CONVERT(bit, 1) THEN NULL ELSE ItemLifespan END
			 , CASE WHEN ProductFlag = CONVERT(bit, 1) THEN NULL ELSE EternityFlag END
			 , CASE WHEN ProductFlag = CONVERT(bit, 1) THEN NULL ELSE ItemExpireDate END
			 , CASE WHEN ProductFlag = CONVERT(bit, 1) THEN NULL ELSE DragonJewelType END
		  FROM T;
	END

	/**_# ??? ?? ?? ?? */
	IF @inyTargetTypeCode = 2
	BEGIN
		IF @xmlAccountIDs IS NOT NULL
		BEGIN
			WITH T AS (
				SELECT T.col.value('@AccountID', 'int') AS AccountID
				  FROM @xmlAccountIDs.nodes('/root/Account') T(col)
			)
			INSERT dbo.EventRewardTargets (EventRewardID, AccountID, CharacterID, DeleteFlag)
			SELECT @intEventRewardID, A.AccountID, NULL, CONVERT(bit, 0)
			  FROM T
				   INNER JOIN dbo.Accounts A WITH (NOLOCK) ON (T.AccountID = A.AccountID);
		END
		ELSE
		BEGIN
			WITH T AS (
				SELECT T.col.value('@AccountName', 'nvarchar(50)') AS AccountName
				  FROM @xmlAccountNames.nodes('/root/Account') T(col)
			)
			INSERT dbo.EventRewardTargets (EventRewardID, AccountID, CharacterID, DeleteFlag)
			SELECT @intEventRewardID, A.AccountID, NULL, CONVERT(bit, 0)
			  FROM T
				   INNER JOIN dbo.Accounts A WITH (NOLOCK) ON (T.AccountName = A.AccountName);
		END
	END
	ELSE IF @inyTargetTypeCode = 4
	BEGIN
		IF @xmlCharacterIDs IS NOT NULL
		BEGIN
			WITH T AS (
				SELECT T.col.value('@CharacterID', 'bigint') AS CharacterID
				  FROM @xmlCharacterIDs.nodes('/root/Character') T(col)
			)
			INSERT dbo.EventRewardTargets (EventRewardID, AccountID, CharacterID, DeleteFlag)
			SELECT @intEventRewardID, C.AccountID, C.CharacterID, CONVERT(bit, 0)
			  FROM T
				   INNER JOIN dbo.Characters C WITH (NOLOCK) ON (T.CharacterID = C.CharacterID)
			 WHERE C.DeleteFlag = 0;
		END
		ELSE
		BEGIN
			WITH T AS (
				SELECT T.col.value('@CharacterName', 'nvarchar(30)') AS CharacterName
				  FROM @xmlCharacterNames.nodes('/root/Character') T(col)
			)
			INSERT dbo.EventRewardTargets (EventRewardID, AccountID, CharacterID, DeleteFlag)
			SELECT @intEventRewardID, C.AccountID, C.CharacterID, CONVERT(bit, 0)
			  FROM T
				   INNER JOIN dbo.Characters C WITH (NOLOCK) ON (T.CharacterName = C.CharacterName)
			 WHERE C.DeleteFlag = 0
			   AND C.CharacterName IS NOT NULL;
		END
	END

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
