USE [dnmembership]
GO
/****** Object:  Table [dbo].[CharacterStarterItems]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[CharacterStarterItems]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[CharacterStarterItems](
	[CharacterStarterItemID] [int] IDENTITY(1,1) NOT NULL,
	[GiftSenderName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[WorldID] [int] NOT NULL,
	[GiftMemo] [nvarchar](100) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[GiftLifeSpan] [int] NOT NULL,
	[ProductID] [int] NOT NULL,
	[ItemIDs] [varchar](max) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[ProvideStartDate] [smalldatetime] NULL,
	[ProvideEndDate] [smalldatetime] NULL,
	[RegisterDate] [smalldatetime] NOT NULL,
 CONSTRAINT [PK_CharacterStarterItems] PRIMARY KEY CLUSTERED 
(
	[CharacterStarterItemID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1] TEXTIMAGE_ON [DataFG1]
END
GO
SET ANSI_PADDING OFF
GO
/****** Object:  Index [IX_NN_CharacterStarterItems_ProvideStartDate_ProvideEndDate]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[CharacterStarterItems]') AND name = N'IX_NN_CharacterStarterItems_ProvideStartDate_ProvideEndDate')
CREATE NONCLUSTERED INDEX [IX_NN_CharacterStarterItems_ProvideStartDate_ProvideEndDate] ON [dbo].[CharacterStarterItems]
(
	[ProvideStartDate] ASC,
	[ProvideEndDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
