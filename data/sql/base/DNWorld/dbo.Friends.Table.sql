USE [dnworld]
GO
/****** Object:  Table [dbo].[Friends]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Friends](
	[CharacterID] [bigint] NOT NULL,
	[FriendCharacterID] [bigint] NOT NULL,
	[FriendMemo] [nvarchar](300) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[FriendGroupID] [int] NULL,
	[RegisterDate] [datetime2](0) NOT NULL,
	[FriendStatusCode] [tinyint] NULL,
 CONSTRAINT [PK_Friends] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[FriendCharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_Friends_FriendCharacterID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_Friends_FriendCharacterID] ON [dbo].[Friends]
(
	[FriendCharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NNI_Friends_FriendGroupID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NNI_Friends_FriendGroupID] ON [dbo].[Friends]
(
	[FriendGroupID] ASC
)
INCLUDE([FriendCharacterID],[FriendMemo]) WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
ALTER TABLE [dbo].[Friends]  WITH CHECK ADD  CONSTRAINT [FK_FriendGroups_Friends_FriendGroupID] FOREIGN KEY([FriendGroupID])
REFERENCES [dbo].[FriendGroups] ([FriendGroupID])
GO
ALTER TABLE [dbo].[Friends] CHECK CONSTRAINT [FK_FriendGroups_Friends_FriendGroupID]
GO
