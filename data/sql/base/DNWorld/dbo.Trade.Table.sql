USE [dnworld]
GO
/****** Object:  Table [dbo].[Trade]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Trade](
	[TradeID] [int] IDENTITY(1,1) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[ItemID] [int] NOT NULL,
	[ItemSerial] [bigint] NOT NULL,
	[ItemLevel] [tinyint] NOT NULL,
	[ItemCount] [smallint] NOT NULL,
	[SkillUsingTypeCode] [tinyint] NOT NULL,
	[Price] [int] NOT NULL,
	[ListPrice] [int] NOT NULL,
	[RegisterCommission] [int] NOT NULL,
	[TradeCommission] [int] NOT NULL,
	[DisplayFlag1] [bit] NOT NULL,
	[DisplayFlag2] [bit] NOT NULL,
	[TradeStatusCode] [tinyint] NOT NULL,
	[TradeExpireDate] [datetime2](0) NOT NULL,
	[RegisterDate] [datetime2](0) NOT NULL,
	[BuyerCharacterID] [bigint] NULL,
	[TradeDate] [datetime2](0) NULL,
	[ItemDurability] [smallint] NULL,
	[RandomSeed] [int] NULL,
	[ItemPotential] [tinyint] NULL,
	[ItemOption] [tinyint] NULL,
	[SealCount] [tinyint] NULL,
	[PremiumFlag] [bit] NOT NULL,
	[PayMethodCode] [tinyint] NULL,
	[ItemPotentialMoveCount] [tinyint] SPARSE  NULL,
	[DragonJewelType] [tinyint] NULL,
	[CharacterName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
 CONSTRAINT [PK_Trade] PRIMARY KEY CLUSTERED 
(
	[TradeID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_Trade_TradeDate]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_Trade_TradeDate] ON [dbo].[Trade]
(
	[TradeDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NNI_Trade_CharacterID_TradeStatusCode]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NNI_Trade_CharacterID_TradeStatusCode] ON [dbo].[Trade]
(
	[CharacterID] ASC,
	[TradeStatusCode] ASC
)
INCLUDE([TradeExpireDate]) WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NNI_Trade_ItemSerial]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NNI_Trade_ItemSerial] ON [dbo].[Trade]
(
	[ItemSerial] ASC
)
INCLUDE([TradeStatusCode]) WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NNI_Trade_RegisterDate]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NNI_Trade_RegisterDate] ON [dbo].[Trade]
(
	[RegisterDate] ASC
)
INCLUDE([ItemID]) WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NNIF_Trade_ItemID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NNIF_Trade_ItemID] ON [dbo].[Trade]
(
	[ItemID] ASC
)
INCLUDE([TradeExpireDate],[PremiumFlag],[ListPrice],[CharacterID],[ItemLevel],[PayMethodCode]) 
WHERE ([TradeStatusCode]=(0))
WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
ALTER TABLE [dbo].[Trade] ADD  CONSTRAINT [DF_Trade_PremiumFlag]  DEFAULT ((0)) FOR [PremiumFlag]
GO
