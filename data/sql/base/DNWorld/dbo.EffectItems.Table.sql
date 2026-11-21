USE [dnworld]
GO
/****** Object:  Table [dbo].[EffectItems]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[EffectItems](
	[EffectItemID] [int] IDENTITY(1,1) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[ItemID] [int] NOT NULL,
	[EffectItemGetCode] [tinyint] NOT NULL,
	[EffectItemGetKey] [bigint] NULL,
	[Price] [int] NOT NULL,
	[EternityFlag] [bit] NOT NULL,
	[EffectItemExpireDate] [smalldatetime] NOT NULL,
	[Property1] [int] NULL,
	[Property2] [int] NULL,
	[Property3] [int] NULL,
	[Property4] [int] NULL,
	[Property5] [int] NULL,
	[RegisterDate] [datetime2](0) NOT NULL,
	[ExpireCompleteFlag] [bit] NOT NULL,
	[ItemSerial] [bigint] NOT NULL
) ON [DataFG1]
GO
/****** Object:  Index [IX_NC_EffectItems_CharacterID]    Script Date: 11/14/2026 8:54:15 AM ******/
CREATE CLUSTERED INDEX [IX_NC_EffectItems_CharacterID] ON [dbo].[EffectItems]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
/****** Object:  Index [PK_EffectItems]    Script Date: 11/14/2026 8:54:37 AM ******/
ALTER TABLE [dbo].[EffectItems] ADD  CONSTRAINT [PK_EffectItems] PRIMARY KEY NONCLUSTERED 
(
	[EffectItemID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_UN_EffectItems_ItemSerial]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_EffectItems_ItemSerial] ON [dbo].[EffectItems]
(
	[ItemSerial] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
ALTER TABLE [dbo].[EffectItems] ADD  CONSTRAINT [DF_EffectItems_ExpireCompleteFlag]  DEFAULT ((0)) FOR [ExpireCompleteFlag]
GO
