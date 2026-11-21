USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListDragonJewel]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 13
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-03-07
description : ?? ???? ?????.
result set : 
	version 1 : ParentItemSerial/bigint, DragonJewelLocationIndex/tinyint, ItemID/int, ItemPotential/tinyint, SequenceNumber/int
	version 2 : ParentItemSerial/bigint, DragonJewelLocationIndex/tinyint, ItemID/int, ItemPotential/tinyint, SequenceNumber/int, SealCount/tinyint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103390 = ??? ??? ???? ????.
history :
12~13 = ORDER BY ??
**/

CREATE PROCEDURE [dbo].[P_GetListDragonJewel]	  
	  @vchItemSerial varchar(max) --// ??? ??? ??? (, ???)	
	, @intVersion int = 1 --// SP?? Version
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @nvcSelectStmt varchar(max);

DECLARE @tblParentItemSerials table (
	ItemSerial bigint NOT NULL,
	SequenceNumber int NOT NULL
);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY	
	/**_# [DragonJewels] ???? SELECT???.*/
	IF @vchItemSerial IS NOT NULL 	
	BEGIN
		INSERT @tblParentItemSerials (ItemSerial, SequenceNumber)
		SELECT CAST (string AS bigint), CAST(seq AS int)
		FROM dbo.TF_Split(@vchItemSerial, ',');
		
		IF @intVersion = 2		
			SELECT DJ.ParentItemSerial, DJ.DragonJewelLocationIndex, DJ.ItemID, DJ.ItemOption, SI.SequenceNumber, MI.SealCount
			FROM dbo.DragonJewels DJ WITH (NOLOCK, INDEX(IX_UNI_DragonJewels_ParentItemSerial_DragonJewelLocationIndex), FORCESEEK)
				 INNER JOIN @tblParentItemSerials SI ON DJ.ParentItemSerial = SI.ItemSerial
				 INNER JOIN dbo.MaterializedItems MI WITH (NOLOCK, INDEX(PK_MaterializedItems), FORCESEEK) ON DJ.MaterializedItemID = MI.MaterializedItemID
			ORDER BY SI.SequenceNumber, DJ.ParentItemSerial;			 
		ELSE 
			SELECT DJ.ParentItemSerial, DJ.DragonJewelLocationIndex, DJ.ItemID, DJ.ItemOption, SI.SequenceNumber
			FROM dbo.DragonJewels DJ WITH (NOLOCK, INDEX(IX_UNI_DragonJewels_ParentItemSerial_DragonJewelLocationIndex), FORCESEEK)
				 INNER JOIN @tblParentItemSerials SI ON DJ.ParentItemSerial = SI.ItemSerial
			ORDER BY SI.SequenceNumber, DJ.ParentItemSerial;			 

		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 103390;
			GOTO ErrorHandler;
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
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO
