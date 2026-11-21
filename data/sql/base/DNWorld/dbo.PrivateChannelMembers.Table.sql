USE [dnworld]
GO
/****** Object:  Table [dbo].[PrivateChannelMembers]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PrivateChannelMembers](
	[CharacterID] [bigint] NOT NULL,
	[PrivateChannelID] [bigint] NOT NULL,
	[CharacterName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[ServerID] [int] NULL,
 CONSTRAINT [PK_PrivateChannelMembers] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[PrivateChannelID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
ALTER TABLE [dbo].[PrivateChannelMembers]  WITH CHECK ADD  CONSTRAINT [FK_PrivateChannels_PrivateChannelMembers_PrivateChannelID] FOREIGN KEY([PrivateChannelID])
REFERENCES [dbo].[PrivateChannels] ([PrivateChannelID])
GO
ALTER TABLE [dbo].[PrivateChannelMembers] CHECK CONSTRAINT [FK_PrivateChannels_PrivateChannelMembers_PrivateChannelID]
GO
