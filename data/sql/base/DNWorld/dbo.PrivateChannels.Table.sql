USE [dnworld]
GO
/****** Object:  Table [dbo].[PrivateChannels]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PrivateChannels](
	[PrivateChannelID] [bigint] IDENTITY(1,1) NOT NULL,
	[PrivateChannelName] [nvarchar](10) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[MasterCharacterID] [bigint] NOT NULL,
	[ChannelPassword] [int] NULL,
 CONSTRAINT [PK_PrivateChannels] PRIMARY KEY CLUSTERED 
(
	[PrivateChannelID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
SET ANSI_PADDING ON
GO
/****** Object:  Index [IX_UN_PrivateChannels_PrivateChannelName]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_PrivateChannels_PrivateChannelName] ON [dbo].[PrivateChannels]
(
	[PrivateChannelName] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
