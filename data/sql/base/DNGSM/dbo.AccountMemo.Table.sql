USE [dngsm]
GO
/****** Object:  Table [dbo].[AccountMemo]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[AccountMemo](
	[AccountMemoID] [int] IDENTITY(1,1) NOT NULL,
	[AccountID] [bigint] NOT NULL,
	[CharacterID] [bigint] NULL,
	[Memo] [nvarchar](2000) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[AdminID] [bigint] NOT NULL,
	[LogDate] [datetime] NOT NULL,
	[MemoTypeID] [tinyint] NOT NULL,
 CONSTRAINT [PK_AccountMemo] PRIMARY KEY CLUSTERED 
(
	[AccountMemoID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_AccountMemo_AccountID]    Script Date: 11/14/2026 8:47:09 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_AccountMemo_AccountID] ON [dbo].[AccountMemo]
(
	[AccountID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_AccountMemo_CharacterID]    Script Date: 11/14/2026 8:47:09 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_AccountMemo_CharacterID] ON [dbo].[AccountMemo]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
