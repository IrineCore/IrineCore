USE [dnworld]
GO
/****** Object:  Table [dbo].[TeacherCharacters]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[TeacherCharacters](
	[TeacherCharacterID] [bigint] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[GenderCode] [tinyint] NULL,
	[TeacherListShowFlag] [bit] NOT NULL,
	[RespectPoint] [int] NOT NULL,
	[SelfIntroduction] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[GraduateCount] [int] NOT NULL,
	[RegisterDate] [datetime2](0) NOT NULL,
	[ConnectFlag] [bit] NOT NULL,
 CONSTRAINT [PK_TeacherCharacters] PRIMARY KEY CLUSTERED 
(
	[TeacherCharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NNIF_TeacherCharacters_RespectPoint]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NNIF_TeacherCharacters_RespectPoint] ON [dbo].[TeacherCharacters]
(
	[RespectPoint] ASC
)
INCLUDE([TeacherCharacterID],[JobCode],[GenderCode],[SelfIntroduction]) 
WHERE ([TeacherListShowFlag]=CONVERT([bit],(1),(0)) AND [ConnectFlag]=CONVERT([bit],(1),(0)))
WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
ALTER TABLE [dbo].[TeacherCharacters] ADD  CONSTRAINT [DF_TeacherCharacters_ConnectFlag]  DEFAULT ((1)) FOR [ConnectFlag]
GO
