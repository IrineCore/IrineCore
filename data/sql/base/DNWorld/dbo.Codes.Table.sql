USE [dnworld]
GO
/****** Object:  Table [dbo].[Codes]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[Codes](
	[codeColumnName] [nvarchar](128) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[codeSN] [tinyint] NOT NULL,
	[description] [nvarchar](100) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[creator] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[createdDate] [datetime] NOT NULL,
 CONSTRAINT [PK_Codes] PRIMARY KEY CLUSTERED 
(
	[codeColumnName] ASC,
	[codeSN] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
ALTER TABLE [dbo].[Codes]  WITH CHECK ADD  CONSTRAINT [FK_CodeKinds_Codes_codeColumnName] FOREIGN KEY([codeColumnName])
REFERENCES [dbo].[CodeKinds] ([codeColumnName])
GO
ALTER TABLE [dbo].[Codes] CHECK CONSTRAINT [FK_CodeKinds_Codes_codeColumnName]
GO
